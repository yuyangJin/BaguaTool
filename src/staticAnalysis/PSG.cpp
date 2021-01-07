#include <stdio.h>
#include <map>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include "CodeObject.h"
#include "CFG.h"
#include "InstructionDecoder.h"
#include "Symtab.h"
#include "LineInformation.h"
#include <string.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <inttypes.h>
#include <stdint.h>
#include <bfd.h>
#include <bfdlink.h>
#include <string>
#include "PSG.h"

using namespace std;
using namespace Dyninst;
using namespace ParseAPI;
//using namespace SymtabAPI;
using namespace InstructionAPI;
typedef bfd_vma VMA;
typedef bfd_signed_vma VMASigned; // useful for offsets

//#define DEBUG_COUT

#ifndef MAX_LOOP_DEPTH
#define MAX_LOOP_DEPTH 2
#endif
#define VMA_MAX ( ~((bfd_vma)(0)) )
#define PTR_TO_BFDVMA(x)         ((bfd_vma)(uintptr_t)(x))
#define BFDVMA_TO_PTR(x, totype) ((totype)(uintptr_t)(x))

#define PTR_TO_VMA(x)          PTR_TO_BFDVMA(x)
#define VMA_TO_PTR(x, totype)  BFDVMA_TO_PTR(x, totype)

unordered_map<Block*, bool> visitedBlock;

unordered_map<Address, string> addr2FuncName; 

map <VMA, VMA> callMap;
unordered_map<string , Node*> func2Node;

void writeTree(Node* node, ostream& fout) {
	if (node && node->removed )
		return;
	//if (node->isAttachedToAnotherNode)
	//	return;
	//for (int i = 0; i < depth; ++i) {
	//	cout << "  ";
	//}
	if (!node) {
		//cout << "null\n";
		return;
	}
	fout << node->id << "\t"
		<< node->type << "\t"
		<< node->childID << "\t"
    << node->numChildren << "\t"
		//<< node->dirID << ":"
		//<< node->fileID << ":"
		<< hex << node->entryAddr << "\t"
    << node->exitAddr << dec << "\n";
	for (auto child: node->children) {
		writeTree(child, fout);
	}
}

void recordAllTrees(Node* root, ostream& fout) {
  //fout << func2Node.size() << '\n';
  unsigned int i = 0;
  for (auto& iter : func2Node){
		if (iter.second && !iter.second->removed){// && !iter.second->isAttachedToAnotherNode){
			//if (!iter.second->removed ){
				//fout<< i++ <<": "<<iter.first<<"\n";
				//writeTree(iter.second,fout);
			//}
      i++;
		}
	}
  fout << i << "\n";
  //fout<<"0 : root\n";
  writeTree(root,fout);
  //unsigned int i = 1;
  for (auto& iter : func2Node){
		if (iter.second && !iter.second->removed && iter.first != "main" ){// && !iter.second->isAttachedToAnotherNode){
			//if (!iter.second->removed ){
				//fout<< i++ <<": "<<iter.first<<"\n";
				writeTree(iter.second,fout);
			//}
		}
	}
}

void printTree(Node* node, int depth) {
	if (node && node->removed )
		return;
			//if (node->isAttachedToAnotherNode)
			//	return;
	for (int i = 0; i < depth; ++i) {
		cout << "  ";
	}
	if (!node) {
		cout << "null\n";
		return;
	}
	cout << node->id << "\t"
		<< node->type << "\t"
		<< node->childID << "\t"
    << node->numChildren << "\t"
		//<< node->dirID << ":"
		//<< node->fileID << ":"
		<< hex << node->entryAddr << "\t"
    << node->exitAddr << dec << "\n";
	for (auto child: node->children) {
		printTree(child, depth + 1);
	}
}



void printFunc2Node(){
	int i = 1;
	for (auto& iter : func2Node){
		if (iter.second){// && !iter.second->isAttachedToAnotherNode){
			if (!iter.second->removed ){
				cout<< i++ <<": "<<iter.first<<"\n";
				printTree(iter.second,0);
			}
		}
	}
}
void generateChildID(Node* node) {
	if (node->childIDGenerated)
		return;
	int id = 0;
	
	for (auto child: node->children) {
		if (child && !child->removed) {
			child->childID = id++;
			generateChildID(child);
		}
	}
	
	node->numChildren = id;
	node->childIDGenerated = true;
}

int generateID(Node* node, int currentID) {
	if (node->idGenerated)
		return currentID;
	node->id = currentID++;
	
	for (auto child: node->children) {
		if (child && !child->removed) {
			currentID = generateID(child, currentID);
		}
	}

	node->idGenerated = true;
	//maxNodeId = (maxNodeId >= currentID ) ? maxNodeId : currentID ;
	return currentID;
}



bool trim(Node* node, int loopDepth) {
	// Return true if this node should not be removed
	if (!node)
		return false;
	//if (node->trimmed)
	//	return !(node->removed);
	if (node->type >= 0 || node->type == CALL_REC_NODE || node->type == CALL_IND_NODE ) {
		node->removed = false;
		node->trimmed = true;
		return true;
	}else if (node->type == LOOP_NODE ){
    loopDepth++;
    if(loopDepth <= MAX_LOOP_DEPTH){
      node->removed = false;
		  node->trimmed = true;
      for (auto child: node->children) {
		    trim(child,loopDepth);
	    }
      return true;
    }
  }

	bool reserved = false;
	for (auto child: node->children) {
		reserved |= trim(child,loopDepth);
	}

	node->removed = !reserved;
	node->trimmed = true;
	return reserved;
}

void expand(Node* node) {
	if (!node || node->expanded)
		return;

	for (auto child: node->children) {
		expand(child);
	}

	if (node->type == CALL_NODE) {
		auto callee = node->funcName;
    if ( func2Node.find(callee) != func2Node.end()){
		  auto calleeNode = func2Node[callee];
		  assert(calleeNode);
		  expand(calleeNode);
		  node->addChild(calleeNode);
		  calleeNode->isAttachedToAnotherNode = true;
    }
	}

	node->expanded = true;
}


int startsWith(string s, string sub){
	return s.find(sub)==0?1:0;
}

/*
void GetLoopInFunc(Function *f){
	// Get all loops in the function 
	vector<Loop*> loops; 
	f->getLoops(loops);

	// Iterate over all loops
	for (auto lit = loops.begin(); lit != loops.end(); ++lit) { Loop *loop = *lit;
		// Get all the entry blocks of the loop
		vector<Block*> entries; 
		loop->getLoopEntries(entries);
		// Get all the blocks in the loop
		vector<Block*> blocks; 
		loop->getLoopBasicBlocks(blocks);
		// Get all the back edges in the loop
		vector<Edge*> backEdges;
		loop->getBackEdges(backEdges);
	}
}


*/


/*
// ***************************************************************************

// Returns nodelist as a list of InlineNodes for the inlined sequence
// at VMA addr.  The front of the list is the outermost frame, back is
// innermost.
//
bool
analyzeAddr(InlineSeqn & nodelist, VMA addr)//, RealPathMgr * realPath)
{
FunctionBase *func, *parent;
bool ret = false;

if (the_symtab == NULL) {
return false;
}
nodelist.clear();

if (the_symtab->getContainingInlinedFunction(addr, func) && func != NULL)
{
ret = true;

parent = func->getInlinedParent();
while (parent != NULL) {
//
// func is inlined iff it has a parent
//
InlinedFunction *ifunc = static_cast <InlinedFunction *> (func);
pair <string, Offset> callsite = ifunc->getCallsite();
string filenm = callsite.first;
//if (filenm != "") { realPath->realpath(filenm); }
long lineno = callsite.second;

// symtab does not provide mangled and pretty names for
// inlined functions, so we have to decide this ourselves
string procnm = func->getName();
string prettynm =
(procnm == "") ? UNKNOWN_PROC : BinUtil::demangleProcName(procnm);

#if DEBUG_INLINE_SEQNS
cout << "\n0x" << hex << addr << dec
<< "  l=" << lineno << "  file:  " << filenm << "\n"
<< "0x" << hex << addr << "  symtab:  " << procnm << "\n"
<< "0x" << addr << dec << "  demang:  " << prettynm << "\n";
#endif

nodelist.push_front(InlineNode(filenm, prettynm, lineno));

func = parent;
parent = func->getInlinedParent();
}
}

return ret;
}
 */



// Comparison functions to sort blocks, edges, loops for more
// deterministic output.

// Sort Blocks by start address, low to high.
	static bool
BlockLessThan(Block * b1, Block * b2)
{
	return b1->start() < b2->start();
}
/*
// Get addr interval by blocks vector;
static  void 
getAddrIntervalByBasicBlocks(vector <Block*> blocks, int & entryAddr, int & exitAddr) {
sort(blocks.begin(), blocks.end(), BlockLessThan);

entryAddr = blocks.begin()->start();
exitAddr = blocks.end()->end();

}
 */
// Returns: the min entry VMA for the loop, or else 0 if the loop is
// somehow invalid.  Irreducible loops have more than one entry
// address.
	static VMA
LoopMinEntryAddr(Loop * loop)
{
	if (loop == NULL) {
		return 0;
	}

	vector <Block *> entBlocks;
	int num_ents = loop->getLoopEntries(entBlocks);

	if (num_ents < 1) {
		return 0;
	}

	VMA ans = VMA_MAX;
	for (int i = 0; i < num_ents; i++) {
		ans = std::min(ans, entBlocks[i]->start());
	}

	return ans;
}

// Sort Loops (from their LoopTreeNodes) by min entry VMAs.
	static bool
LoopTreeLessThan(LoopTreeNode * n1, LoopTreeNode * n2)
{
	return LoopMinEntryAddr(n1->loop) < LoopMinEntryAddr(n2->loop);
}

void doLoopTree(LoopTreeNode * ltNode,int depth, Node* parentNode){
	if (ltNode == NULL) {
		return ;
	}
	//Loop *loop = ltNode->loop;

	//for (int i = 0; i < depth ; i++)
	//	printf("  ");
	//printf("Loop : %s\n",ltNode->name());
	// process the children of the loop tree
	vector <LoopTreeNode *> clist = ltNode->children;
	std::sort(clist.begin(), clist.end(), LoopTreeLessThan);

	for (uint i = 0; i < clist.size(); i++) {
		vector<Block*> blocks;
		clist[i] ->loop -> getLoopBasicBlocks(blocks);
		sort(blocks.begin(), blocks.end(), BlockLessThan);

		Address entryAddr = blocks[0]->start();
		Address exitAddr = blocks[blocks.size() - 1]->end();
		//getAddrIntervalByBasicBlocks(blocks, entryAddr, exitAddr);

    auto loopNode = new Node(LOOP_NODE, entryAddr-8, exitAddr-8);
    parentNode->addChild(loopNode);

#ifdef DEBUG_COUT 
		for (int i = 0; i < depth ; i++)
			printf("  ");
		cout<<"Loop : " << clist[i]->name() << " addr : " << hex << entryAddr <<" - " << exitAddr <<dec <<endl;		
#endif
		//int numCall = clist[i] -> numCallees();
		//vector <Function* > callees = clist[i] -> getCallees();
		//for (int j = 0 ; j < numCall ; j++){
		//	for (int i = 0; i < depth + 1 ; i++)
		//        	printf("  ");
		//	printf("Call : %s\n",clist[i]->getCalleeName(j));
		//}
		doLoopTree(clist[i], depth+1, loopNode);
		if(clist[i] -> numCallees() >0){	
			int j = 0;
			auto bit = blocks.begin();
			for( ; bit != blocks.end(); ++bit) {

				Block *b = *bit;
				if(!visitedBlock[b]){
					visitedBlock[b] = true;
					auto it = b->targets().begin();
					for( ; it != b->targets().end(); ++it) {
						if((*it)->type() == CALL){
              entryAddr = (*it)->src()->last();
              exitAddr = (*it)->src()->last();
              Node* callNode = nullptr;
              if(startsWith(addr2FuncName[ callMap[(*it)->src()->last()]], "MPI_")){
                callNode = new Node(MPI_NODE, entryAddr, exitAddr);
              }else if(addr2FuncName[ callMap[(*it)->src()->last()] ] == string("")){
                callNode = new Node(CALL_IND_NODE, entryAddr, exitAddr);
              }else{
                callNode = new Node(CALL_NODE, addr2FuncName[ callMap[(*it)->src()->last()] ], entryAddr, exitAddr);
              }
              loopNode->addChild(callNode);
#ifdef DEBUG_COUT 
							for (int i = 0; i < depth + 1 ; i++)
								printf("  ");
              
							cout << "Call : " << addr2FuncName[ callMap[(*it)->src()->last()] ]
								<< " addr : " << hex << (*it)->src()->last() <<" - " << (*it)->src()->last() <<dec <<endl;;
#endif
							j++;
						}
					}
				}
			}
		}
	}	

	return ;
}

int main(int argc, char *argv[]){
	map<Address, bool> seen;
	vector<Function *> funcs;
	SymtabCodeSource *sts;
	CodeObject *co;
	unordered_map<Address, string> addr2funcname;

  //SymtabAPI::Symtab *obj = nullptr;
    
  //auto root = node

	// Create a new binary code object from the filename argument
	sts = new SymtabCodeSource( argv[1] ); 
	co = new CodeObject( sts );
  
  //bool err = SymtabAPI::Symtab::openFile(obj, argv[1]);
  //cout << (obj == nullptr) <<endl;
	// Parse the binary
	co->parse();
	//set <Address> coveredFuncs;
	//map <VMA, VMA> callMap;

  

	// Print the control flow graph
	const CodeObject::funclist& all = co->funcs();
	auto fit = all.begin();
	for(int i = 0; fit != all.end(); ++fit, i++) {
		Function * func = * fit;
		addr2FuncName[func->addr()] = func->name();
		//Function * func = fit->second->func;
		const Function::edgelist & elist = func->callEdges();
		for (auto eit = elist.begin(); eit != elist.end(); ++eit) {
			VMA src = (*eit)->src()->last();
			VMA targ = (*eit)->trg()->start();
			callMap[src] = targ;
		}
	}

	long num = 0;
	fit = all.begin();
	for(int i = 0; fit != all.end(); ++fit, i++) {	
		Function * func = * fit;
		Address entry_addr = func->addr();
		string funcName = func->name();	
		//if(startsWith(funcName,"_")){
		//	continue;
		//}

		num++;

		const ParseAPI::Function::blocklist & blist = func->blocks();
		vector <Block *> bvec;
		for (auto bit = blist.begin(); bit != blist.end(); ++bit) {
			Block * block = *bit;
			bvec.push_back(block);
		}

		sort(bvec.begin(), bvec.end(), BlockLessThan);

		Address entryAddr = bvec[0]->start();
		Address exitAddr = bvec[bvec.size() - 1]->last();		

    auto funcNode = new Node(FUNC_NODE, entryAddr, exitAddr);
    func2Node[func->name()] = funcNode;

		//BlockSet visited;
		//if(func-> region() == NULL);
		//	continue;
  /*
    if (func->name() == "main"){
    SymtabAPI::Module *mod=nullptr;
    //vector<SymtabAPI::Module *> &mods;
    //obj->getAllModules(mods);


    obj->findModuleByName(mod, "test.c");
    if (!mod){
      cout << func->name() << " mod error"<<endl;
    }
    vector<SymtabAPI::Statement *> lines;
    SymtabAPI::LineInformation *info = mod->getLineInformation();
    
    vector<Dyninst::SymtabAPI::AddressRange>ranges;
    mod->getAddressRanges(ranges,"test.c",30);
    for(auto range : ranges){
      cout <<hex<< range.first << " - " << range.second<<dec<< endl;
    } 
    */
    /*  
    Offset addressInRange = ranges[0].first;
    bool getSource = info->getSourceLines(addressInRange,lines);
    if (getSource){
    for (auto line : lines){
      cout<<line->first<<line->second<<line->getColumn()<<endl;
    }
    }*/
    //}

#ifdef DEBUG_COUT 
		cout <<"Function : "<< func->name()<< " addr : " << hex << entry_addr << "/" <<entryAddr<<" - "<<exitAddr<< dec << endl;
#endif

		/*
		// compute the inline seqn for the call site for this func, if
		// there is one.
		//Inline::InlineSeqn prefix;
		auto call_it = callMap.find(entry_addr);

		if (call_it != callMap.end()) {
		//analyzeAddr(prefix, call_it->second);
		}
		// see if this function is entirely contained within another
		// function (as determined by its entry block).  if yes, then we
		// don't parse the func.  but we still use its blocks for gaps,
		// unless we've already added the containing func.
		//
		int num_contain = func->entry()->containingFuncs();
		bool add_blocks = true;

		if (num_contain > 1) {
		vector <Function *> funcVec;
		func->entry()->getFuncs(funcVec);

		// see if we've already added the containing func
		for (auto fit = funcVec.begin(); fit != funcVec.end(); ++fit) {
		Address entry = (*fit)->addr();

		if (coveredFuncs.find(entry) != coveredFuncs.end()) {
		add_blocks = false;
		break;
		}
		}
		}
		// skip duplicated function, blocks already added
		if (! add_blocks) {
		DEBUG_CFG("\nskipping duplicated function:  '" << func->name() << "'\n");
		continue;
		}

		// basic blocks for this function.  put into a vector and sort by
		// start VMA for deterministic output.
		vector <Block *> bvec;
		//BlockSet visited;

		const Function::blocklist & blist = func->blocks();

		for (auto bit = blist.begin(); bit != blist.end(); ++bit) {
		Block * block = *bit;
		bvec.push_back(block);
		visited[block] = false;
		}

		sort(bvec.begin(), bvec.end(), BlockLessThan);
		 */
		// traverse the loop (Tarjan) tree
		LoopTreeNode * ltNode = func->getLoopTree();
		doLoopTree(ltNode, 1, funcNode);	

		// process any blocks not in a loop
		/*
		   for (auto bit = bvec.begin(); bit != bvec.end(); ++bit) {
		   Block * block = *bit;
		   if (! visited[block]) {
		   doBlock(env, ginfo, func, visited, block, root);
		   }
		   }
		 */

		//auto edgeList = func->callEdges();

		//		InstructionDecoder decoder(func->isrc()->getPtrToInstruction(func->addr()), InstructionDecoder::maxInstructionLength, func->region()->getArch());
		//		Instruction::Ptr inst;	

		int j = 0;
		auto bit = bvec.begin();
		for( ; bit != bvec.end(); ++bit) {
			Block *b = *bit;
			if(!visitedBlock[b]){
				visitedBlock[b] = true;
				auto it = b->targets().begin();
				for( ; it != b->targets().end(); ++it) {
					if((*it)->type() == CALL){
						
						//cout << "Call : " << decoder.decode((unsigned char *)func->isrc()->getPtrToInstruction((*it)->src()->start())).format()
            entryAddr = (*it)->src()->last();
            exitAddr = (*it)->src()->last();
            Node* callNode = nullptr;
            if(startsWith(addr2FuncName[ callMap[(*it)->src()->last()]], "MPI_")){
              callNode = new Node(MPI_NODE, entryAddr, exitAddr);
            }else if(addr2FuncName[ callMap[(*it)->src()->last()] ] == string("")){
              callNode = new Node(CALL_IND_NODE, entryAddr, exitAddr);
            } else{
              callNode = new Node(CALL_NODE, addr2FuncName[ callMap[(*it)->src()->last()] ], entryAddr, exitAddr);
            }
            funcNode->addChild(callNode);
#ifdef DEBUG_COUT   
            for (int i = 0; i < 1 ; i++)
							printf("  ");
                     
						cout << "Call : " << addr2FuncName[ callMap[(*it)->src()->last()] ]
							<< " addr : " << hex << (*it)->src()->last() <<" - " << (*it)->src()->last() <<dec <<endl;;
#endif
						j++;
					}
				}
			}
		}

	}
#ifdef DEBUG_COUT
  cout << "==============================" <<endl;
  printFunc2Node();
  cout << "==============================" <<endl;
#endif

  Node* root = func2Node["main"];
  expand(root);

#ifdef DEBUG_COUT
  printTree(root,0);
  cout << "==============================" <<endl;
#endif

  trim(root,0);

#ifdef DEBUG_COUT
  printTree(root,0);
  cout << "==============================" <<endl;
#endif

  generateID(root, 0);
	generateChildID(root);

  // Write tree to a file
  string outputFilename = string(argv[1]) + ".psg";
	ofstream fout(outputFilename.c_str());

	if (!fout.good()) {
		cout << "Failed to open output file\n";
		return true;
	}

#ifdef DEBUG_COUT
  printTree(root,0);
#endif

  recordAllTrees(root, fout); // first print root

  fout.close();
  //cout << "==============================" <<endl;
}
