#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
//#include <unistd.h>
//#include <bfd.h>
//#include <bfdlink.h>

#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <CFG.h>
#include <CodeObject.h>
#include <InstructionDecoder.h>
#include <LineInformation.h>
#include <Symtab.h>

#include "baguatool.h"
#include "core/pag.h"
#include "static_analysis.h"

using namespace std;
using namespace Dyninst;
using namespace ParseAPI;
// using namespace SymtabAPI;
using namespace InstructionAPI;

namespace baguatool::graph_sd {

StaticAnalysis::StaticAnalysis(char *binary_name) { this->sa = new StaticAnalysisImpl(binary_name); }

StaticAnalysis::~StaticAnalysis() { delete sa; }

void StaticAnalysis::IntraProceduralAnalysis() { sa->IntraProceduralAnalysis(); }
void StaticAnalysis::InterProceduralAnalysis() { sa->InterProceduralAnalysis(); }
void StaticAnalysis::CaptureProgramCallGraph() { sa->CaptureProgramCallGraph(); }
void StaticAnalysis::DumpAllFunctionGraph() { sa->DumpAllFunctionGraph(); }
void StaticAnalysis::GetBinaryName() { sa->GetBinaryName(); }
/*
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
                << hex << node->entry_addr << "\t"
    << node->exit_addr << dec << "\n";
        for (auto child: node->children) {
                writeTree(child, fout);
        }
}

void recordAllTrees(Node* root, ostream& fout) {
  //fout << this->func_2_graph.size() << '\n';
  unsigned int i = 0;
  for (auto& iter : this->func_2_graph){
                if (iter.second && !iter.second->removed){// &&
!iter.second->isAttachedToAnotherNode){
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
  for (auto& iter : this->func_2_graph){
                if (iter.second && !iter.second->removed && iter.first != "main"
){// && !iter.second->isAttachedToAnotherNode){
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
                << hex << node->entry_addr << "\t"
    << node->exit_addr << dec << "\n";
        for (auto child: node->children) {
                printTree(child, depth + 1);
        }
}


void printFunc2Node(){
        int i = 1;
        for (auto& iter : this->func_2_graph){
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
        if (node->type >= 0 || node->type == CALL_REC_NODE || node->type ==
CALL_IND_NODE ) {
                node->removed = false;
                node->trimmed = true;
                return true;
        }else if (node->type == core::LOOP_NODE ){
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



*/

int startsWith(string s, string sub) { return s.find(sub) == 0 ? 1 : 0; }

// Comparison functions to sort blocks, edges, loops for more
// deterministic output.

// Sort Blocks by start address, low to high.
static bool BlockLessThan(Block *b1, Block *b2) { return b1->start() < b2->start(); }

// Returns: the min entry VMA for the loop, or else 0 if the loop is
// somehow invalid.  Irreducible loops have more than one entry
// address.
static VMA LoopMinEntryAddr(Loop *loop) {
    if (loop == NULL) {
        return 0;
    }

    vector<Block *> entBlocks;
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
static bool LoopTreeLessThan(LoopTreeNode *n1, LoopTreeNode *n2) {
    return LoopMinEntryAddr(n1->loop) < LoopMinEntryAddr(n2->loop);
}

// Capture a Program Call Graph (PCG)
void StaticAnalysisImpl::CaptureProgramCallGraph() {
    // core::ProgramAbstractionGraph* pcg = new
    // core::ProgramAbstractionGraph("program_call_graph");

    // Get function list
    const CodeObject::funclist &func_list = this->co->funcs();

    auto fit = func_list.begin();
    auto endfit = func_list.end();

    // Traverse through all functions
    for (int i = 0; fit != endfit; ++fit, i++) {
        Function *func = *fit;
        this->addr_2_func_name[func->addr()] = func->name();
        const Function::edgelist &elist = func->callEdges();

        // Traverse through all fuunction calls in this function
        for (auto eit = elist.begin(); eit != elist.end(); ++eit) {
            VMA src = (*eit)->src()->last();
            VMA targ = (*eit)->trg()->start();
            this->call_graph_map[src] = targ;
        }
    }
}

// Capture function call structure in this function but not in the loop
void StaticAnalysisImpl::ExtractCallStructure(core::ProgramAbstractionGraph *func_struct_graph, vector<Block *> &bvec,
                                              int parent_id) {
    auto bit = bvec.begin();

    // Traverse through all blocks
    for (; bit != bvec.end(); ++bit) {
        Block *b = *bit;

        // If block is visited, it means it is inside the loop
        if (!this->visited_block_map[b]) {
            this->visited_block_map[b] = true;
            auto inst_iter = b->targets().begin();
            auto inst_end_iter = b->targets().end();

            // Traverse through all instructions
            for (; inst_iter != inst_end_iter; ++inst_iter) {
                // Only analyze CALL type instruction
                if ((*inst_iter)->type() == CALL) {
#ifdef DEBUG_COUT
                    cout << "Call : "
                         << decoder.decode((unsigned char *)func->isrc()->getPtrToInstruction((*it)->src()->start()))
                                .format()
#endif
                                    Address entry_addr = (*inst_iter)->src()->last();
                    Address exit_addr = (*inst_iter)->src()->last();
                    string call_name = this->addr_2_func_name[this->call_graph_map[entry_addr]];
                    int call_vertex_id = 0;

                    // Add a CALL vertex, including MPI_CALL, INDIRECT_CALL, and CALL
                    if (startsWith(call_name, "MPI_") || startsWith(call_name, "_MPI_") ||
                        startsWith(call_name, "mpi_") || startsWith(call_name, "_mpi_")) {  // MPI communication calls
                        call_vertex_id = func_struct_graph->AddVertex();
                        func_struct_graph->SetVertexBasicInfo(call_vertex_id, core::MPI_NODE, call_name.c_str());
                        // call_vertex_id = new Node(core::MPI_NODE, entry_addr, exit_addr);
                    } else if (call_name == string("")) {  // Function calls that are not
                                                           // analyzed at static analysis
                        call_vertex_id = func_struct_graph->AddVertex();
                        func_struct_graph->SetVertexBasicInfo(call_vertex_id, core::CALL_IND_NODE, call_name.c_str());
                        // call_vertex_id = new Node(core::CALL_IND_NODE, entry_addr,
                        // exit_addr);
                    } else {  // Common function calls
                        call_vertex_id = func_struct_graph->AddVertex();
                        func_struct_graph->SetVertexBasicInfo(call_vertex_id, core::CALL_NODE, call_name.c_str());
                        // call_vertex_id = new Node(core::CALL_NODE, call_name, entry_addr,
                        // exit_addr);
                    }

                    func_struct_graph->SetVertexDebugInfo(call_vertex_id, entry_addr, exit_addr);

                    // Add an edge
                    func_struct_graph->AddEdge(parent_id, call_vertex_id);
#ifdef DEBUG_COUT
                    for (int i = 0; i < 1; i++) cout << "  ";
                    cout << "Call : " << call_name << " addr : " << hex << entry_addr << " - " << exit_addr << dec
                         << endl;
                    ;
#endif
                }
            }
        }
    }
}

void StaticAnalysisImpl::ExtractLoopStructure(core::ProgramAbstractionGraph *func_struct_graph, LoopTreeNode *loop_tree,
                                              int depth, int parent_id) {
    if (loop_tree == NULL) {
        return;
    }

    // process the children of the loop tree
    vector<LoopTreeNode *> child_loop_list = loop_tree->children;
    std::sort(child_loop_list.begin(), child_loop_list.end(), LoopTreeLessThan);

    for (uint i = 0; i < child_loop_list.size(); i++) {
        vector<Block *> blocks;
        child_loop_list[i]->loop->getLoopBasicBlocks(blocks);
        sort(blocks.begin(), blocks.end(), BlockLessThan);

        Address entry_addr = blocks[0]->start();
        Address exit_addr = blocks[blocks.size() - 1]->end();

        string loop_name = child_loop_list[i]->name();

        int loop_vertex_id = func_struct_graph->AddVertex();
        func_struct_graph->SetVertexBasicInfo(loop_vertex_id, core::LOOP_NODE, loop_name.c_str());
        func_struct_graph->SetVertexDebugInfo(loop_vertex_id, entry_addr - 8, exit_addr - 8);

        func_struct_graph->AddEdge(parent_id, loop_vertex_id);

#ifdef DEBUG_COUT
        for (int i = 0; i < depth; i++) cout << "  ";
        cout << "Loop : " << loop_name << " addr : " << hex << entry_addr << " - " << exit_addr << dec << endl;
#endif

        this->ExtractLoopStructure(func_struct_graph, child_loop_list[i], depth + 1, loop_vertex_id);
        if (child_loop_list[i]->numCallees() > 0) {
            this->ExtractCallStructure(func_struct_graph, blocks, loop_vertex_id);
        }
    }

    return;
}

// Extract structure graph for each fucntion
void StaticAnalysisImpl::IntraProceduralAnalysis() {
    // Get function list
    const CodeObject::funclist &func_list = this->co->funcs();

    auto fit = func_list.begin();
    auto endfit = func_list.end();

    // Traverse through all functions
    for (int i = 0; fit != endfit; ++fit, i++) {
        Function *func = *fit;
        Address entry_addr = func->addr();
        string func_name = func->name();

        // Create a graph for each function
        core::ProgramAbstractionGraph *func_struct_graph = new core::ProgramAbstractionGraph();
        func_struct_graph->GraphInit(func_name.c_str());
        this->func_2_graph[func_name] = func_struct_graph;

        const ParseAPI::Function::blocklist &blist = func->blocks();
        vector<Block *> bvec;
        for (auto bit = blist.begin(); bit != blist.end(); ++bit) {
            Block *block = *bit;
            bvec.push_back(block);
        }

        sort(bvec.begin(), bvec.end(), BlockLessThan);
        entry_addr = bvec[0]->start();
        Address exit_addr = bvec[bvec.size() - 1]->last();

        // Create root vertex in the graph
        int func_vertex_id = func_struct_graph->AddVertex();
        func_struct_graph->SetVertexBasicInfo(func_vertex_id, core::FUNC_NODE, func_name.c_str());
        func_struct_graph->SetVertexDebugInfo(func_vertex_id, entry_addr, exit_addr);

// Add DebugInfo attributes
// auto func_vertex_id = new Node(core::FUNC_NODE, entry_addr, exit_addr);

#ifdef DEBUG_COUT
        cout << "Function : " << func_name << " addr : " << hex << entry_addr << "/" << entry_addr << " - " << exit_addr
             << dec << endl;
#endif

        // Capture loop structure in this function
        // Traverse through the loop (Tarjan) tree
        LoopTreeNode *loop_tree = func->getLoopTree();
        this->ExtractLoopStructure(func_struct_graph, loop_tree, 1, func_vertex_id);

        // Capture function call structure in this function but not in the loop
        this->ExtractCallStructure(func_struct_graph, bvec, func_vertex_id);
    }
}

void StaticAnalysisImpl::DumpFunctionGraph(core::ProgramAbstractionGraph *func_struct_graph, const char *file_name) {
    func_struct_graph->DumpGraph(file_name);
}

void StaticAnalysisImpl::DumpAllFunctionGraph() {
    string dir_name = string(getcwd(NULL, 0)) + string("/") + string(this->binary_name) + string(".pag");

    printf("%s\n", dir_name.c_str());
    if (access(dir_name.c_str(), F_OK)) {
        if (mkdir(dir_name.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) {
            printf("mkdir failed\n");
            return;
        }
    }

    // Get function list
    const CodeObject::funclist &func_list = this->co->funcs();

    auto fit = func_list.begin();
    auto endfit = func_list.end();

    // Traverse through all functions
    for (int i = 0; fit != endfit; ++fit, i++) {
        Function *func = *fit;
        string func_name = func->name();
        core::ProgramAbstractionGraph *func_struct_graph = this->func_2_graph[func_name];
        string file_name = string("./") + string(this->binary_name) + string(".pag/") + func_name + string(".gml");
        this->DumpFunctionGraph(func_struct_graph, file_name.c_str());
    }
}

void StaticAnalysisImpl::GetBinaryName() {
    //   return this->binary_name;
}

// int CallerCallee(core::ProgramAbstractionGraph* func_struct_graph, const int
// vertex_id){
//   if (func_struct_graph->GetVertexType(vertex_id) == CALL){

//   }
// }

void StaticAnalysisImpl::InterProceduralAnalysis() {}

// void StaticAnalysis::InterProceduralAnalysis(core::ProgramAbstractionGraph*
// func_struct_graph) {

//   func_struct_graph->Dfs(dfs_callback, NULL);

//   // if (!node || node->expanded)
// 	// 	return;

// 	// for (auto child: node->children) {
// 	// 	expand(child);
// 	// }

// 	// if (node->type == core::CALL_NODE) {
// 	// 	auto callee = node->funcName;
//   //   if ( this->func_2_graph.find(callee) != this->func_2_graph.end()){
// 	// 	  auto calleeNode = this->func_2_graph[callee];
// 	// 	  assert(calleeNode);
// 	// 	  expand(calleeNode);
// 	// 	  node->addChild(calleeNode);
// 	// 	  calleeNode->isAttachedToAnotherNode = true;
//   //   }
// 	// }

// 	// node->expanded = true;
// }
}