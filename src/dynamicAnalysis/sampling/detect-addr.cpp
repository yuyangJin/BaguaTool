/*
---------------------------------
version 1.0.1
1. Replace line number by address
//2. Sampling count of nodes: exclude -> include
---------------------------------
*/
//#include "PSG.h"
#include <stdio.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <cassert>
#include <chrono>
#include <stack>
#include <algorithm>
#include <vector>
#include <queue>
#include <string>
#include <mpi.h>

using namespace std;

//#define DEBUG_PRINT
#define MAX_CALL_STACK 100
#define MAX_MPI_INFO 1000000

enum NodeType {
  COMPUTE_NODE = -9,
  //COMBINE = -8,
  CALL_IND_NODE = -7,
  CALL_REC_NODE = -6,
  CALL_NODE = -5,
  FUNC_NODE = -4,
  //COMPOUND = -3,
  //BRAN_NODE = -2,
  LOOP_NODE = -1,
  MPI_NODE = 1,
};

class Node {
	public:
		int id = -1;
    int newid = -1;
		int type = -10;
		int numChildren = 0;
		int childID = -1;
    int entryAddr = -1;
    int exitAddr = -1;
		int dirID = -1;
		int fileID = -1;
		int entryLineNum = -1;
		int exitLineNum = -1;
		Node* parent = nullptr;
		vector<Node*> children;

		int sampleCount = 0;
		double sumTime = 0;
		
		bool comNodeExpanded = false;
		int curPid = -1;
		int commInfoSize = 0;
		struct MPIINFO *commInfo = nullptr;

		Node() {}

		Node(int id, int type, int childID, int numChildren, int entryAddr, int exitAddr): // int dirID , int fileID, int entryLineNum, int exitLineNum):
			id(id), type(type), childID(childID), numChildren(numChildren), entryAddr(entryAddr), exitAddr(exitAddr){
				children.resize(static_cast<unsigned int>(numChildren));
			}
    Node(int id, int type, int numChildren, int entryAddr, int exitAddr): //int dirID , int fileID, int entryLineNum, int exitLineNum):
			id(id), type(type), numChildren(numChildren), entryAddr(entryAddr), exitAddr(exitAddr){}

};



typedef struct logstruct{
	char funcType = 0; // Y for entry , T for exit , L for LATCH , C for COMBINE
	int childId = 0;
	int id = 0;
	int nodeType = 0; // 
	long long time = 0;
}LOG;


LOG logline;


//int mpiRank;

int maxId = 0;

static Node* root = nullptr;
static Node* now = nullptr;
static Node* last = nullptr;
static vector<Node*> trees;
static vector<string> dirs, files;
static stack<int> lineStack;
static string popline;
//struct MPIINFO mpiInfo[MAX_MPI_INFO];
unsigned int mpiInfoPointer = 0;

static void readTree(Node*& node, istream& in, unsigned int depth) {
	int id, type, childID, numChildren, entryAddr, exitAddr; //, dirID , fileID, entryLineNum, exitLineNum; 
  if (!in.eof()){
    //in >> dec >> id >> type >> childID >> numChildren  >> hex >> entryAddr >> exitAddr >> dec;//dirID >> fileID >> entryLineNum >> exitLineNum;
    in >> id >> type >> childID >> numChildren  >> entryAddr >> exitAddr ;//dirID >> fileID >> entryLineNum >> exitLineNum;
	  //printf ("%d,%d,%d,%d,%x,%x\n",id, type, childID, numChildren, entryAddr, exitAddr);
    node = new Node(id, type, childID, numChildren, entryAddr, exitAddr);//dirID , fileID, entryLineNum, exitLineNum);
	  maxId = ( maxId > id ) ? maxId : id ;
	  for (auto& child: node->children) {
		  readTree(child, in, depth + 1);
		  child->parent = node;
	  }
  }
}

static Node* copyTree(Node* node){

	//copy node to newnode
  maxId++;
	Node* newnode = new Node(maxId, node->type, node->numChildren, node->entryAddr, node->exitAddr); //node->dirID, node->fileID, node->entryLineNum, node->exitLineNum);
	newnode -> comNodeExpanded = node -> comNodeExpanded; 

  //copy child to newnode
	int i = 0;
	for (auto child: node->children) {
		Node* newchild = copyTree(child);
		newnode->children.push_back(newchild);
		newchild->parent = newnode;     
		i++;   
	}
	return newnode;

}

void generateChildID(Node* node) {
	//if (node->childIDGenerated)
	//	return;
	int id = 0;
	
	for (auto child: node->children) {
		if (child) {
			child->childID = id++;
			generateChildID(child);
		}
	}
	
	node->numChildren = id;
	//node->childIDGenerated = true;
}


static Node* findTreeRootWithId(int id){
	for(auto& node: trees){
		if(node->id == id)
			return node;
	}

}

static Node* findTreeRootWithLineNum(int dirID, int fileID, int lineNum){
	for(auto& node: trees){
		if(node->dirID == dirID && node->fileID == fileID && node->exitAddr >= lineNum && node->entryAddr <= lineNum)
			return node;
	}
}

static Node* findTreeRootWithAddr(int popAddr){
	for(auto& node: trees){
		if(node->exitAddr >= popAddr && node->entryAddr <= popAddr)
			return node;
	}
}

static Node* findCallIndirectNodeWithId(Node* node, int id){

	if(id == node->id && node->type == CALL_IND_NODE){
		return node;
	}

	for (auto child: node->children){
		Node* retNode = findCallIndirectNodeWithId(child, id);
		if(retNode){
			return retNode;
		}
	}

	return nullptr;
}

static Node* findMpiNodeWithId(Node* node, int id){

        if(id == node->id && node->type >= 0){
                return node;
        }

        for (auto child: node->children){
                Node* retNode = findMpiNodeWithId(child, id);
                if(retNode){
                        return retNode;
                }
        }

        return nullptr;
}

static void printTree(Node* node, int depth) {
  for (int i = 0; i < depth; i++){
    cout << " ";
  }
	cout << node->id << " "
		<< node->type << " "
    << node->childID << " "
		//<< node->children.size() << " "
    << node->numChildren << " "
		//<< node->dirID << " "
		//<< node->fileID << " "
		<< node->entryAddr << " "
		<< node->exitAddr << " "
		<< node->sampleCount << " "
		//<< node->sumTime << " "
		<< "\n";
	node->sampleCount = 0;
	node->sumTime = 0;
	for (auto child: node->children)
		printTree(child, depth+1);

}

static void writeTree(Node* node, ostream& out) {
	out << node->id << " "
		<< node->type << " "
    << node->childID << " "
		<< node->children.size() << " "
		//<< node->dirID << " "
		//<< node->fileID << " "
		<< node->entryAddr << " "
		<< node->exitAddr  << " "
		<< node->sampleCount << " "
		<< node->sumTime << " "
		<< "\n";
	node->sampleCount = 0;
	node->sumTime = 0;
	for (auto child: node->children)
		writeTree(child, out);

}

/*
----------------------
Read .psg file to get program struct graph
----------------------
*/
static void initialize(string binName) {
	ifstream inputStream(binName + string(".psg"));
	if (!inputStream.good()) {
		cout << "Failed to open "<< binName + string(".psg")<<"\n";
		exit(1);
	}
	int numTrees = 0;
	inputStream >> numTrees;
	//cout <<numTrees<<": before read tree!\n";
	int totalMaxDepth = 0;
	for(int i = 0;i < numTrees; i++){
		Node *treeRoot = nullptr;
		readTree(treeRoot, inputStream, 0);
		trees.push_back(treeRoot);
	}
	root = trees[0];

  inputStream.close();

  /*
  // Read dir and file stream, the dirs and files list below the .psg file is the subset of CALLPATH0.TXT-df.
  ifstream inputDfStream(string("CALLPATH0.TXT-df"));
	if (!inputDfStream.good()) {
		cerr << "Failed to open CALLPATH0.TXT-df\n";
		exit(1);
	}

	unsigned int cnt = 0;
	inputDfStream >> cnt;
	dirs.resize(cnt);
	for (int i = 0; i < cnt; ++i)
		inputDfStream >> dirs[i];
	inputDfStream >> cnt;
	files.resize(cnt);
	for (int i = 0; i < cnt; ++i)
		inputDfStream >> files[i];
  */
}


static void finalize(int mpiRank) {
	cerr<< "ENTER FINALIZE"<<endl;
	ofstream outputStream(string("stat") + to_string(mpiRank) + string(".txt"));
	if (!outputStream.good()) {
		cerr << "Failed to open output file"<<endl;
		return;
	}
	writeTree(root, outputStream);

  /*
	outputStream << dirs.size() << "\n";
	for (auto& s: dirs)
		outputStream << s << "\n";
	outputStream << files.size() << "\n";
	for (auto& s: files)
		outputStream << s << "\n";
  */
  outputStream.close();
	return ;
}



static void debug_print(Node* now, int type, int id, string funcName) {
#ifdef DEBUG_PRINT
	if (now && now->dirID >= 0)
		cerr << funcName << "\t" << type << "\t" << id << "\t" << dirs[now->dirID] << "/" << files[now->fileID] << ":" << now->entryAddr << endl;
	else
		cerr << funcName << "\t" << type << "\t" << id << "\n";
#endif
}


void entryHandler(int id, int type) {
	if (type == CALL_IND_NODE){
		//cerr<<"CALL_IND_NODE\n";
		now = findCallIndirectNodeWithId(root, id);
		debug_print(now, type, id, __func__);
		return;
	}

	if(type == FUNC_NODE && now && now->type == CALL_IND_NODE){ // exclude main function
		for(auto child : now->children){
			if(child->id == id){
				return ;
			}
		}
		now -> numChildren ++;
		//Node * indirectFuncNode = copyTree(findTreeRootWithId(id));
		Node * indirectFuncNode = findTreeRootWithId(id);
		now -> children.push_back(indirectFuncNode);
		now -> numChildren = now -> children.size();
		indirectFuncNode -> parent = now;
		now = indirectFuncNode;
		debug_print(now, type, id, __func__);
		return ;
	}

	debug_print(now, type, id, __func__);

}

void exitHandler(int id, int type) {
	now = nullptr;
	debug_print(now, type, id, __func__);
}


void expandComputeNode(Node* node){
	if(node->comNodeExpanded){
		return;
	}
	//cerr<<__LINE__<<endl;
	int curLineNum = node-> entryAddr;
	queue<Node*> tempComNodeQueue;
	//cerr <<endl << node->id <<" "<< node->type<<" " << node->entryAddr<<" " <<node->exitAddr<<endl;
	for(auto child: node-> children){
		//cerr<<__LINE__<<endl;
		if(child && child->type != COMPUTE_NODE && (node->type == FUNC_NODE ||node->type == LOOP_NODE)){ //only add computing node to function and loop node
			//cerr<<__LINE__<<endl;
			//cerr << child->id <<" "<< child->type<<" " << child->entryAddr<<" " <<child->exitAddr<<endl;
			if(child->entryAddr > curLineNum){
				//cerr<<__LINE__<<endl;
				maxId ++;
				//Node *comNode = new Node(maxId, COMPUTE_NODE, 0, node->dirID, node->fileID, curLineNum , child->entryAddr - 1);
        Node *comNode = new Node(maxId, COMPUTE_NODE, 0, curLineNum , child->entryAddr - 1);
				tempComNodeQueue.push(comNode);
				//cerr << "push "<<comNode->id << " : "<< comNode->entryAddr <<" - "<<comNode->exitAddr<<endl;

			}
			//curLineNum = child->exitAddr;
		}
		if(child && child->type != COMPUTE_NODE){
			//cerr<<__LINE__<<endl;
			expandComputeNode(child);
			//cerr<<__LINE__<<endl;
		}
		if(child)
			curLineNum = child->exitAddr + 1;
	}
	if(node->type == FUNC_NODE ||node->type == LOOP_NODE){ //only add computing node to function and loop node
		//cerr<<__LINE__<<endl;
		if(node->exitAddr > curLineNum){
			//cerr<<__LINE__<<endl;
			maxId ++;
			//Node *comNode = new Node(maxId, COMPUTE_NODE, 0, node->dirID, node->fileID, curLineNum , node->exitAddr );
      Node *comNode = new Node(maxId, COMPUTE_NODE, 0, curLineNum , node->exitAddr - 1);
			tempComNodeQueue.push(comNode);
			//cerr << "push "<< comNode->id << " : "<< comNode->entryAddr <<" - "<<comNode->exitAddr<<endl;
		}
	}

	while(!tempComNodeQueue.empty()){
		node->children.push_back(tempComNodeQueue.front());
		tempComNodeQueue.pop();
	}

	node->numChildren = node->children.size();
	node->comNodeExpanded = true;
}

/*
Swap the values of elements in Node x and y.
*/
static void swapNode(Node* x,Node* y){
	int id = x->id;
	int type = x->type;
	int numChildren = x->numChildren;
  int childID = x->childID;
  int entryAddr = x->entryAddr;
  int exitAddr = x->exitAddr;
	//int dirID = x->dirID;
	//int fileID = x->fileID;
	//int entryLineNum = x->entryLineNum;
	//int exitLineNum = x->exitLineNum;
	int sampleCount = x->sampleCount;
	double sumTime = x->sumTime;
	//Node* parent = x->parent;
	//vector<Node*> *children = x->children;  vector swap
	bool comNodeExpanded = x->comNodeExpanded;
	//int curPid = -1;
	//int commInfoSize = 0;
	//struct MPIINFO *commInfo = nullptr;

	x->id = y->id;
	x->type = y->type;
	x->numChildren = y->numChildren;
  x->childID = y->childID;
  x->entryAddr = y->entryAddr;
  x->exitAddr = y->exitAddr;
	//x->dirID = y->dirID;
	//x->fileID = y->fileID;
	//x->entryLineNum = y->entryLineNum;
	//x->exitLineNum = y->exitLineNum;
	x->sampleCount = y->sampleCount;
	x->sumTime = y->sumTime;
  x->comNodeExpanded = y->comNodeExpanded;
	x->children.swap(y->children);

	y->id = id;
	y->type = type;
	y->numChildren = numChildren;
  y->childID = childID;
  y->entryAddr = entryAddr;
  y->exitAddr = exitAddr;
	//y->dirID = dirID;
	//y->fileID = fileID;
	//y->entryLineNum = entryLineNum;
	//y->exitLineNum = exitLineNum;
	y->sampleCount = sampleCount;
	y->sumTime = sumTime;
  y->comNodeExpanded = comNodeExpanded;

	for (auto childx: x->children) {
		childx->parent = x;
	}
	for (auto childy: y->children) {
		childy->parent = y;
	}
}


static void sortByLineNum(Node* node){
	if(node->numChildren > 1){
		for (int i = 0; i < node->children.size() ; i++) {
			int k = i;
			for (int j = i; j < node->children.size(); j++) {
				if(node->children[j]->entryAddr < node->children[k]->entryAddr )
					k = j ;
			}
			swapNode(node->children[k],node->children[i]);
			node->children[i]->childID = i;
		}
	}else if(node->numChildren == 1){
		node->children[0]->parent = node;
		node->children[0]->childID = 0;
	}
	for (auto child : node->children){
		sortByLineNum(child);
	}
}


void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c)
{
	std::string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while(std::string::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2-pos1));

		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if(pos1 != s.length())
		v.push_back(s.substr(pos1));
}


Node* findNodeBFS(Node* node, int popAddr){

	//cerr<<"findNodeBFS: "<< node->id << endl;
	for(auto child : node->children){
		if(child->entryAddr <= popAddr && popAddr <= child->exitAddr ){
			
			if(child->children.size() == 0){
        //cout << "no child return : "<< child->id << " type : " << child->type<<endl;
				return child;
			}
			//cout <<child->id << " : " <<child->type <<endl;
      /*if(child -> type == CALL_IND_NODE){
        //First attach the copy of callee function
        //Then return
        return child;
      }else*/ 
      if(child -> type >= 0 || child -> type == CALL_NODE || child -> type == CALL_IND_NODE || child -> type == CALL_REC_NODE){
				//cout << "return : "<< child->id << " type : " << child->type<<endl;
				return child;
			}else if(child -> type == LOOP_NODE || child->type == FUNC_NODE ){ //|| child->type == BRANCH || child->type == COMPOUND){
				return findNodeBFS(child,popAddr);
			}else{
				//cout << "return : "<< child->id << " type : " << child->type<<endl;
				return child;
			}
		}
	}

	for(auto child : node->children){
		return findNodeBFS(child,popAddr);
	}

	//if node is leaf ,return nullptr
	//cerr << "return : NULL"<<endl;
	return nullptr;
}

bool hasAttachedCallee(Node* node, int popAddr){
  for(auto child : node->children){
    //cout << "child->dirID: " << child->dirID<< "child->fileID: "<<child->fileID<< "node->exitAddr: "<<node->exitAddr << " " <<node->entryAddr << "popLineNum: "<< popLineNum<<endl;
    if (child->exitAddr >= popAddr && child->entryAddr <= popAddr){
      return true;
    }
  }
  return false;
}

Node* updateLeafWithLineStack(Node* node){
	//pop if popFlag is true
	if(lineStack.empty()){
		return node;
	}
	int popAddr = lineStack.top();
	lineStack.pop();

	//find node -> popFlag = true

  /*
	vector<string> v;
	SplitString(popLine,v," ");

  int popDirId = stoi(v[0]);
  int popFileId = stoi(v[1]);
  int popLineNum = stoi(v[2]);

  */

  Node* findNode = nullptr;

  //If line number is ??, we record as -1 in the file, skip it and pop the next LineNum
  if (popAddr < 0x40000){
    return updateLeafWithLineStack(node);
  }

	//cerr<<"popDirId: "<< popDirId << " find fileid: "<<popFileId << " line: "<<popLineNum<< endl;

	findNode = findNodeBFS(node, popAddr);


	if (!findNode){
    //cout<<"*****************NOT FOUND**********"<<endl;
		return nullptr;
	}

  //If indirect call node is found
  if (findNode -> type == CALL_IND_NODE || (findNode -> type == CALL_NODE && findNode -> numChildren == 0) ){
    if(!lineStack.empty()){
      //First read the next line
	    popAddr = lineStack.top();

      //cerr<<"popDirId: "<< popDirId << " find fileid: "<<popFileId << " line: "<<popLineNum<< hasAttachedCallee(findNode,popDirId,popFileId,popLineNum)<< endl;
      //cerr<<"findNode dir: "<< popDirId << " find fileid: "<<popFileId << " line: "<<popLineNum<< hasAttachedCallee(findNode,popDirId,popFileId,popLineNum)<< endl;
      if (popAddr >= 0x40000){
        if(!hasAttachedCallee(findNode, popAddr)){

          //Find the tree root of indirect callee 
          Node* indCalleeNode = findTreeRootWithAddr(popAddr);
          //cerr<<"indCalleeNode dir: "<< indCalleeNode->dirID << " indCalleeNode fileid: "<<indCalleeNode->fileID << "indCalleeNode line: "<<indCalleeNode->entryAddr<< " "<<indCalleeNode->exitAddr<< endl;
          //expand computation node for the callee function
          sortByLineNum(indCalleeNode);
	        expandComputeNode(indCalleeNode);
	        sortByLineNum(indCalleeNode);
          //cerr<<"findNode dir: "<< findNode->dirID << " findNode fileid: "<<findNode->fileID << "findNode line: "<<findNode->entryAddr<< " "<<findNode->exitAddr<< endl;
          //cerr<<"indCalleeNode dir: "<< indCalleeNode->dirID << " indCalleeNode fileid: "<<indCalleeNode->fileID << "indCalleeNode line: "<<indCalleeNode->entryAddr<< " "<<indCalleeNode->exitAddr<< endl;
          //attach the copy of the callee function to the indirect call
          Node* newNode = copyTree(indCalleeNode);
          //cerr<<"newNode dir: "<< newNode->dirID << " newNode fileid: "<<newNode->fileID << "newNode line: "<<newNode->entryAddr<< " "<<newNode->exitAddr<< endl;
          findNode -> children.push_back(newNode);
          indCalleeNode -> parent = findNode;
          indCalleeNode -> childID = findNode -> numChildren;
          findNode -> numChildren ++ ;
					return updateLeafWithLineStack(newNode);
        }
      }
    }        
  }
	//if(findNode->type == CALL_REC || findNode -> type >= 0 || findNode -> type == COMPUTE_NODE){
	if(!findNode->numChildren){
		return findNode;
	}

	return updateLeafWithLineStack(findNode);

}


/*
Push call path of each line to lineStack, and return sample count
line:
0 0 19 < 0 0 45 < 0 0 68 < 1 1 -1 < | 1
*/
int getLineStack(istream& in){
	string lineStr;
  int sampleCountNum = 0;
  if(getline(in,lineStr)){
    if(lineStr.find('|') == string::npos){ //Error : if | is not found 
    }else{

      vector<string> v;
	    SplitString(lineStr,v,"|");
      int sampleCountNum = stoi(v[1]);
      //cout << "sampleCountNum:" << sampleCountNum << endl ;

      vector<string> v1;
      SplitString(v[0],v1," ");

      for (std::vector<std::string>::iterator iter = v1.begin(); iter != v1.end(); ++iter){
        int tempAddr = (int)strtoll((*iter).c_str(), NULL, 16);
        //cout<< hex << tempAddr <<dec <<" " ;
        lineStack.push(tempAddr);
      }
      //cout <<endl;

      //pop the empty address
      lineStack.pop();

      if (!lineStack.empty()){
      //the top address is not in 'main', start from the second address.
      lineStack.pop();
      }

      return sampleCountNum;

      /*

      //Return the position of '|'.
      int orPointer = lineStr.rfind('|'); 
      //Get the sampling count of the call path. 
      string sampleCountStr = lineStr.substr(orPointer + 1,lineStr.length() - (orPointer + 1));
      //cout << "sampleCountStr:" << sampleCountStr ;
      int sampleCountNum = stoi(sampleCountStr);
      //cout << "sampleCountNum:" << sampleCountNum << endl ;
      //Return the position of '<'.
      int hyphenPointer = -1, lastHyphenPointer = -1;
      hyphenPointer = lineStr.find('<',0);
      //cout << "hyphenPointer: " <<hyphenPointer<<endl;
      //Keep pushing if < is found in the rest of string
      while (hyphenPointer != string::npos){
        string lineNumStr = lineStr.substr(lastHyphenPointer + 1,(hyphenPointer - (lastHyphenPointer + 1)));
        //cout << "hyphenPointer: " <<hyphenPointer << " lastHyphenPointer:"<<lastHyphenPointer<<endl;
        //cout<< lineNumStr <<endl;
        lineStack.push(lineNumStr);
        lastHyphenPointer = hyphenPointer + 1;
        hyphenPointer = lineStr.find('<',lastHyphenPointer);
      }
      return sampleCountNum;
      */

      /*
      emmm.. I find that the SplitString function is written before.
      */


    }

  }else{
    //Read line error
    //cout << "Read line error" <<endl;
  }
  return 0;
}


void updateLeafTimePercent(Node* node, int totalSampleCount){

	//if(node->numChildren == 0)
	node->sumTime = (double)node->sampleCount / (double)totalSampleCount ;
	for(auto child : node-> children){
		updateLeafTimePercent(child,totalSampleCount);
	}

}

void clearNodePerfData(Node* node){
  node->sampleCount = 0;
	node->sumTime = 0;
  for(auto child : node-> children){
		clearNodePerfData(child);
	}
}


/*
void readCommStream(ifstream &in){
	while(!in.eof()){
		int id , setSize, dataSize, count, i;
		in >> id >>setSize >> dataSize >> count;
		int * set =(int*)malloc(sizeof(int)* setSize);
		if(setSize > 0){
			for(i = 0 ; i < setSize; i ++){
				in >> set[i];
			}
		}
		bool repeatFlag = false;
		for (i = 0;i<mpiInfoPointer;i++){
			if(id == mpiInfo[i].id ){
			if(setSize == mpiInfo[i].commSetSize){
				int j = 0;
				repeatFlag = true;
				for (j = 0 ; j < setSize ; j++ ){
					if(mpiInfo[i].commSet[j] != set[j]){
						repeatFlag = false;
						break;
					}
				}
				if(repeatFlag == false){
					continue;
				}
				if(dataSize == mpiInfo[i].dataSize){
					repeatFlag = true;
					break;
				}else{
					repeatFlag = false;
					continue;
				}
			}
			}
		}

		if(repeatFlag == true){
			mpiInfo[i].count += count;
		}else{
			mpiInfo[mpiInfoPointer].id = id;
			mpiInfo[mpiInfoPointer].commSetSize = setSize;
			mpiInfo[mpiInfoPointer].commSet = set;
			mpiInfo[mpiInfoPointer].dataSize = dataSize;
			mpiInfo[mpiInfoPointer].count = 1;
			mpiInfoPointer++;
		}
	}
}

*/

int main(int argc, char**argv){
  //Read .psg file
  initialize(argv[1]);  
  //printTree(root,0);
  
	//for(int i = 0;i < nprocs; i++){
  //Sort the child node of each node by line number (prepare for expand compute node)
	sortByLineNum(root);
  //cout << "======================"<<endl;
  //printTree(root,0);
  //finalize(i);
  //Expand compute node between each node
	expandComputeNode(root);
  //cout << "======================"<<endl;
  //printTree(root,0);
  //finalize(i);
	sortByLineNum(root);
  //cout << "======================"<<endl;
  //printTree(root,0);
	int nprocs = atoi(argv[2]);
	int i = 0;
  
	while(i < nprocs){
    //Clear performance data of each node
    clearNodePerfData(root);


    //Read callpath and sampling count
		ifstream callPathStream((string("SAMPLE")+to_string(i) +string(".TXT")));
		if (!callPathStream.good()) {
			cout << "Failed to open sample.txt\n";
			exit(1);
		}
    /*
    ifstream dirStream((string("CALLPATH")+to_string(i) +string(".TXT-df")));
		if (!dirStream.good()) {
			cerr << "Failed to open callpath.txt-df\n";
			exit(1);
		}
    */
		int totalSampleCount = 0;
		while(!callPathStream.eof()){
      //push the callpath line to linestack
			int curSampleCount = getLineStack(callPathStream);


			if( !lineStack.empty() ){
        //pop from linstack to do traversal in the graph.
				Node* node = updateLeafWithLineStack(root);
				if(node){
					//		cerr<<totalSampleCount<<" : "<< node-> id << endl;
					node->sampleCount += curSampleCount;	
				}
				//	else{cerr<<totalSampleCount<< " :" <<endl;}

				totalSampleCount += curSampleCount;
			}


			//如果函数栈因为遇到一些CALL_REC等节点类型，提前返回了，要把函数栈POP完。
      //If linestack stops popping due to CALL_REC or other reasons 
      //and linestack is not empty, need to clear the linestack
			while(!lineStack.empty() ){
				lineStack.pop();
			}

			//totalSampleCount++;
		}

		callPathStream.close();



		updateLeafTimePercent(root,totalSampleCount);
    

		/*
		ifstream commStream((string("COMM")+to_string(i) +string(".TXT")));
		if (!commStream.good()) {
			cerr << "Failed to open comm.txt\n";
			exit(1);
		}
		
		mpiInfoPointer = 0;
		
		readCommStream(commStream);

		cout << mpiInfoPointer <<endl;

		for (int j = 0; j < mpiInfoPointer; j++){
			Node* findNode = findMpiNodeWithId(root, mpiInfo[j].id);
			if(findNode){
			if(findNode->curPid == i){
				findNode->commInfoSize += 1;
			}else{
				findNode->curPid = i;
				findNode->commInfoSize = 1;
			}
			if(findNode->commInfo){
				findNode->commInfo = (struct MPIINFO *)realloc(findNode->commInfo,findNode->commInfoSize * sizeof(struct MPIINFO));
			}else{
				findNode->commInfo = (struct MPIINFO *)malloc(sizeof(struct MPIINFO));
			}
			findNode->commInfo[findNode->commInfoSize - 1].id = mpiInfo[j].id;
			findNode->commInfo[findNode->commInfoSize - 1].commSetSize = mpiInfo[j].commSetSize;
			findNode->commInfo[findNode->commInfoSize - 1].dataSize = mpiInfo[j].dataSize;
			findNode->commInfo[findNode->commInfoSize - 1].count = mpiInfo[j].count;
			findNode->commInfo[findNode->commInfoSize - 1].commSet = mpiInfo[j].commSet;
			}
		}
		//117
		Node* findNode = findMpiNodeWithId(root,atoi(argv[2]));
		for(int j = 0 ; j < findNode->commInfoSize; j++){
			cout << findNode->type << " "<< findNode->commInfo[j].commSetSize << " " << findNode->commInfo[j].dataSize << " " << findNode->commInfo[j].count<<endl;
			for(int k = 0 ;k < findNode->commInfo[j].commSetSize; k++){
				cout <<findNode->commInfo[j].commSet[k] << " ";
			}
			cout <<endl;
		}
		*/
    generateChildID(root);
		finalize(i);
		i++;
	}
  
  //printTree(root,0);
  //finalize(0);
	//}
}

