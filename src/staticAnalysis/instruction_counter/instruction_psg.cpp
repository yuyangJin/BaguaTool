#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <set>
#include <string>
#include <algorithm>
#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_process.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_function.h"
#include "BPatch_flowGraph.h"
#include "Instruction.h" 
#include "Expression.h"
#include "Operand.h"
using namespace std;
using namespace Dyninst;
using namespace InstructionAPI;

// these macros can be used for colorful output
#define TPRT_NOCOLOR "\033[0m"
#define TPRT_RED "\033[1;31m"
#define TPRT_GREEN "\033[1;32m"
#define TPRT_YELLOW "\033[1;33m"
#define TPRT_BLUE "\033[1;34m"
#define TPRT_MAGENTA "\033[1;35m"
#define TPRT_CYAN "\033[1;36m"
#define TPRT_REVERSE "\033[7m"

#define LOG_INFO(fmt, ...) fprintf(stderr,TPRT_GREEN fmt TPRT_NOCOLOR, __VA_ARGS__);
#define LOG_ERROR(fmt, ...) fprintf(stderr,TPRT_RED fmt TPRT_NOCOLOR, __VA_ARGS__);
#define LOG_WARN(fmt, ...) fprintf(stderr,TPRT_MAGENTA fmt TPRT_NOCOLOR, __VA_ARGS__);
#define LOG_LINE fprintf(stderr,TPRT_BLUE "line=%d\n" TPRT_NOCOLOR, __LINE__);
//


// Create an instance of class BPatch
BPatch bpatch;
// Different ways to perform instrumentation

#define INS_TYPE_NUM 9

unordered_map<BPatch_basicBlock*, bool> visitedBlock;

FILE *fp = NULL;

int InsTypeCount[INS_TYPE_NUM] = {0};

char InsType[INS_TYPE_NUM][20] = {
    "LOAD",
    "STORE",
    "Add",
    "Sub",
    "DoubleMul",
    "IntMul",
    "DoubleDiv",
    "IntDiv",
    "OtherIns"};

typedef enum {
    LOAD,
    STORE,
    Add,
    Sub,
    DoubleMul,
    IntMul,
    DoubleDiv,
    IntDiv,
    OtherIns
} Defined_instruction_t; 

int startsWith(string s, string sub){
  return s.find(sub)==0?1:0;
}

int endsWith(string s,string sub){
  return s.rfind(sub)==(s.length()-sub.length())?1:0;
}
void cleanInsNum(){
	for (int i = 0; i < INS_TYPE_NUM ; i++){
    InsTypeCount[i] = 0;
    //fprintf(stderr,"%d ",InsTypeCount[i]);
  }
}

void printInsNum(){
  for (int i = 0; i < INS_TYPE_NUM ; i++){
    LOG_INFO("%s : %d \n", InsType[i], InsTypeCount[i])
    fprintf(fp,"%d ",InsTypeCount[i]);
  }

  fprintf(fp," \n");
}



void getInstructionType(InstructionAPI::Instruction insn){
  InstructionAPI::Operation OP = insn.getOperation();
#ifdef DEBUG
  cout << OP.format() ;    
#endif
  string operationStr =  OP.format();

  vector <InstructionAPI::Operand> operands;
  insn.getOperands(operands);
#ifdef DEBUG
  for ( auto iter = operands.begin (); iter != operands.end ();++iter ) {
    cout <<" {"<< (*iter).format(Arch_x86_64,0) << "} ";
  }
#endif

  // Recognize Memory Access Type only 
  if (  insn.readsMemory()||insn.writesMemory() ) {
    //first use 
    //vector <InstructionAPI::Operand> operands;
    //insn.getOperands(operands);
    int oprand_id = 0;
    for ( auto iter = operands.begin (); iter != operands.end ();++iter ) {
      string memAccessStr = (*iter).format(Arch_x86_64,0);
      //int num = count(memAccessStr.find('('),memAccessStr.end(),',');
      int num = count(memAccessStr.begin(),memAccessStr.end(),',');
      if (num == 2 || (memAccessStr.find('(') != memAccessStr.npos && memAccessStr.find(')') != memAccessStr.npos) ){
      //if(num == 2){
        if(operationStr == "mov"){
          if (oprand_id == 0){
            InsTypeCount[STORE] ++;
          }else{
            InsTypeCount[LOAD] ++;
          }
        }else if(operationStr == "movsd"){
          //need to split to float and double
          if (oprand_id == 0){
            InsTypeCount[STORE] ++;
          }else{
            InsTypeCount[LOAD] ++;
          }
        }
      }
      oprand_id ++;
    }
  } else if(startsWith(operationStr,"add")){
    if (operationStr == "add"){
      InsTypeCount[Add] ++;
    }else if (operationStr == "addsd"){
      InsTypeCount[Add] ++;
    }
  } else if(startsWith(operationStr,"sub")){
    if (operationStr == "sub"){
      InsTypeCount[Sub] ++;
    }else if (operationStr == "subsd"){
      InsTypeCount[Sub] ++;
    }
  } else if(startsWith(operationStr,"mul") || endsWith(operationStr,"mul")){
    if (operationStr == "imul"){
      InsTypeCount[IntMul] ++;
    }else if (operationStr == "mulsd"){
      InsTypeCount[DoubleMul] ++;
    }
  } else if(startsWith(operationStr,"div")){
    if (operationStr == "div"){
      InsTypeCount[IntDiv] ++;
    }else if (operationStr == "divsd"){
      InsTypeCount[DoubleDiv] ++;
    }
  } else{
    InsTypeCount[OtherIns] ++;
  }
  return ;
}

void doLoopTree(BPatch_loopTreeNode * ltNode,int depth){
  if (ltNode == NULL) {
		return ;
	}
  vector <BPatch_loopTreeNode *> clist = ltNode->children;
  for (uint i = 0; i < clist.size(); i++) {
    //recursive call doLoopTree
    doLoopTree(clist[i], depth+1);

    //clear the instruction count array 
    cleanInsNum();

    std::vector<BPatch_basicBlock*> blocks;
    //clist[i]->loop->getLoopBasicBlocksExclusive(blocks);
    clist[i]->loop->getLoopBasicBlocks(blocks);
    //clist[i]->getAllBasicBlocks(blocks);
    for (auto block_iter = blocks.begin(); block_iter != blocks.end(); ++block_iter) {
      BPatch_basicBlock* block = *block_iter;
      if(!visitedBlock[block]){
        visitedBlock[block] = true;
        std::vector<InstructionAPI::Instruction> insns;
        block->getInstructions(insns);
        for (auto insn_iter = insns.begin(); insn_iter != insns.end(); ++insn_iter) {
            InstructionAPI::Instruction insn = *insn_iter;
            //Defined_instruction_t type = getInstructionType(insn);
            getInstructionType(insn);
#ifdef DEBUG
            //std::cout << "[" <<InsType[type] << "]" << std::endl;
            std::cout << std::endl;
#endif
            //InsTypeCount[type] ++;
        }
      }
    }
    LOG_WARN("%s\n",clist[i]->name())
    //printf("88 %s\n",clist[i]->name());
    fprintf(fp,"L %s\n",clist[i]->name());
    printInsNum();

  }

}

int binaryAnalysis(BPatch_addressSpace* app, const char* funcname) {
    BPatch_image* appImage = app->getImage();
    int insns_access_memory = 0;
    std::vector<BPatch_function*> functions;
    appImage->findFunction(funcname, functions);
    if (functions.size() == 0) {
        fprintf(stderr, "No function InterestingProcedure\n");
        return insns_access_memory;
    } else if (functions.size() > 1) {
        fprintf(stderr, "More than one InterestingProcedure; using the first one\n");
    }
    BPatch_flowGraph* fg = functions[0]->getCFG();
    // traverse the loop (Tarjan) tree
    BPatch_loopTreeNode * ltNode = fg->getLoopTree();
    doLoopTree(ltNode, 1);	

    
    return insns_access_memory;
}
int main(int argc, const char *argv[]) {
 // Set up information about the program to be instrumented
    const char* progName = argv[1];
    //const char** instrumentedFuncName = argv[2];
    //char* instrumentedFuncName;
    //int progPID = 42;
    //const char* progArgv[] = {"test2", "-h", NULL};
    
    //Use BPatch_* classes to initialize
    //BPatch bpatch;
    BPatch_addressSpace *app = bpatch.openBinary(progName, true);
    //bool flag = false; 
    //flag = app->loadLibrary("liblib.so");
    //if (!flag){
    //  std::cout<<"open shard library liblib.so faild" << std::endl;
    //}
    
    BPatch_image* image = app->getImage();

    char output_name[100] = {0};
    strcat(output_name, progName);
    strcat(output_name, ".asm");
    //sscanf(output_name, "%s.asm", progName);
    fp = fopen(output_name,"w+");
    for (int i = 2; i < argc; i++){
        const char * instrumentedFuncName = argv[i];
        fprintf(fp,"F %s\n", instrumentedFuncName);
        int memAccesses = binaryAnalysis(app, instrumentedFuncName);
    }

    fclose(fp);

    //fprintf(stderr, "Found %d memory accesses\n", memAccesses);
    //printInsNum();
    
}
