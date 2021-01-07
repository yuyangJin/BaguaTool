#ifndef LOOPCG_H
#define LOOPCF_H

#include "CodeObject.h"
#include "CFG.h"
#include "InstructionDecoder.h"
#include <string>

using namespace std;
using namespace Dyninst;
using namespace ParseAPI;
using namespace InstructionAPI;

enum NodeType {
  //COMPUTE_NODE = -9,
  //COMBINE = -8,
  CALL_IND_NODE = -7,
  CALL_REC_NODE = -6,
  CALL_NODE = -5,
  FUNC_NODE = -4,
  //COMPOUND = -3,
  //BRANCH = -2,
  LOOP_NODE = -1,
  MPI_NODE = 1,
};

class Node {
  public:
    int id = -1;
    int type;
    //BasicBlock* bb;
    string funcName;
    //CallInst* callInst;

    //For address range
    Address entryAddr = 0;
    Address exitAddr = 0;

    bool expanded = false;
    bool trimmed = false;
    bool removed = false;
    //bool compNodeExpanded = false;
    bool idGenerated = false;
    bool childIDGenerated = false;
    //bool locationInfoCollected = false;
    //bool instrumented = false;
    bool isAttachedToAnotherNode = false;
    
    // The index of this node in its parent's children array
    // Generated after trimming
    int childID = 0;
    int numChildren = 0;

    int dirID = -1;
    int fileID = -1;
    //int lineNum = -1;

    vector<Node*> children;

    Node(NodeType type, string funcName = nullptr, Address entryAddr = 0, Address exitAddr = 0):
      type(static_cast<int>(type)), funcName(funcName), entryAddr(entryAddr), exitAddr(exitAddr) {}

    Node(NodeType type, Address entryAddr = 0, Address exitAddr = 0):
      type(static_cast<int>(type)), entryAddr(entryAddr), exitAddr(exitAddr) {}

    //Node(NodeType type, Address entryAddr = 0, Address exitAddr = 0):
    //  funcName(funcName), entryAddr(entryAddr), exitAddr(exitAddr) {}

    //Node(int funcId, BasicBlock* bb = nullptr, Function* func = nullptr, CallInst* callInst = nullptr):
    //  type(funcId), bb(bb), func(func), callInst(callInst) {}

    void addChild(Node* child){
      children.push_back(child);
      numChildren++;
    }
};

#endif