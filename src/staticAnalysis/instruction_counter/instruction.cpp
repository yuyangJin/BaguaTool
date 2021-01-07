#include <stdio.h>
#include <iostream>
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

int InsTypeCount[17] = {0};

char InsType[17][20] = {"DoubleMemoryAccess",
    "FloatMemoryAccess",
    "IntMemoryAccess",
    "DoubleAdd",
    "DoubleSub",
    "DoubleMul",
    "DoubleDiv",
    "FloatAdd",
    "FloatSub",
    "FloatMul",
    "FloatDiv",
    "IntAdd",
    "IntSub",
    "IntMul",
    "IntDiv",
    "OtherIns"};

typedef enum {
    DoubleMemoryAccess,
    FloatMemoryAccess,
    IntMemoryAccess,
    DoubleAdd,
    DoubleSub,
    DoubleMul,
    DoubleDiv,
    FloatAdd,
    FloatSub,
    FloatMul,
    FloatDiv,
    IntAdd,
    IntSub,
    IntMul,
    IntDiv,
    OtherIns
} Defined_instruction_t; 

int startsWith(string s, string sub){
  return s.find(sub)==0?1:0;
}

int endsWith(string s,string sub){
  return s.rfind(sub)==(s.length()-sub.length())?1:0;
}

void printInsNum(){
  for (int i = 0; i <= 15 ; i++){
    LOG_INFO("%s : %d \n", InsType[i], InsTypeCount[i])
    //fprintf(stderr,"%d ",InsTypeCount[i]);
  }

  //fprintf(stderr," \n");
}

Defined_instruction_t getInstructionType(InstructionAPI::Instruction insn){
  InstructionAPI::Operation OP = insn.getOperation();
  //cout << OP.format() ;    
  string operationStr =  OP.format();
  // Recognize Memory Access Type only 
  if (  insn.readsMemory()||insn.writesMemory() ) {
    //first use 
    vector <InstructionAPI::Operand> operands;
    insn.getOperands(operands);
    for ( auto iter = operands.begin (); iter != operands.end ();++iter ) {
      //cout <<" {"<< (*iter).format(Arch_x86_64,0) << "} ";
      string memAccessStr = (*iter).format(Arch_x86_64,0);
      //int num = count(memAccessStr.find('('),memAccessStr.end(),',');
      //int num = count(memAccessStr.begin(),memAccessStr.end(),',');
      if (memAccessStr.find('(') != memAccessStr.npos && memAccessStr.find(')') != memAccessStr.npos){
      //if(num == 2){
        if(operationStr == "mov"){
          return IntMemoryAccess;
        }else if(operationStr == "movsd"){
          //need to split to float and double
          return DoubleMemoryAccess;
        }
      }
    }
    //cout << endl;
    /*
    startsWith(OP.format(), string("mov"))
    Float

    //insns_access_memory++;
                std::cout << "*" ; 
                std::vector <InstructionAPI::Operand> operands;
                insn.getOperands(operands);
                for ( auto iter = operands.begin (); iter != operands.end ();++iter ) {
                  if ( (*iter).isRead() ){
                    std::set<Expression::Ptr> memAccessors;
                    iter->addEffectiveReadAddresses(memAccessors);
                    //std::set<RegisterAST::Ptr> regsRead;
                    //(*iter).getReadSet(regsRead);
                    for(std::set<Expression::Ptr>::iterator mem=memAccessors.begin() ;mem!=memAccessors.end();mem++){
                      //Expression tmp = *mem; 
                      Result tmp = (*mem)->eval();
                      cout<< "--<" <<tmp.format()<< ">--";
                    }
                    std::cout << " is read" ; 
                  }
                  if ( (*iter).isWritten() ){
                    //std::set<RegisterAST::Ptr> regsRead;
                    //(*iter).getReadSet(regsRead);
                    //for (auto regRead:regsRead){
                    //  cout << regRead.ty
                    //}
                    std::cout << " is written" ; 
                  }
                  //cout <<" Registers used for operand "<<(*iter).getValue() <<" " << (*iter).format(Arch_x86_64,0) << endl;
                //    //iter指向寄存器
                }
            } else{
              //InstructionAPI::Operation OP = insn.getOperation();
              //std::cout << OP.format() << std::endl;
            }
    */
  } 
  if(startsWith(operationStr,"add")){
    if (operationStr == "add"){
      return IntAdd;
    }else if (operationStr == "addsd"){
      return DoubleAdd;
    }
  } else if(startsWith(operationStr,"sub")){
    if (operationStr == "sub"){
      return IntSub;
    }else if (operationStr == "subsd"){
      return DoubleSub;
    }
  } else if(startsWith(operationStr,"mul") || endsWith(operationStr,"mul")){
    if (operationStr == "imul"){
      return IntMul;
    }else if (operationStr == "mulsd"){
      return DoubleMul;
    }
  } else if(startsWith(operationStr,"div")){
    if (operationStr == "div"){
      return IntDiv;
    }else if (operationStr == "divsd"){
      return DoubleDiv;
    }
  }
  return OtherIns;
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
    std::set<BPatch_basicBlock*> blocks;
    fg->getAllBasicBlocks(blocks);
    for (auto block_iter = blocks.begin(); block_iter != blocks.end(); ++block_iter) {
        BPatch_basicBlock* block = *block_iter;
        std::vector<InstructionAPI::Instruction> insns;
        block->getInstructions(insns);
        for (auto insn_iter = insns.begin(); insn_iter != insns.end(); ++insn_iter) {
            InstructionAPI::Instruction insn = *insn_iter;
            Defined_instruction_t type = getInstructionType(insn);
            //std::cout << "[" <<InsType[type] << "]" << std::endl;
            InsTypeCount[type] ++;
        }
    }
    return insns_access_memory;
}
int main(int argc, const char *argv[]) {
 // Set up information about the program to be instrumented
    const char* progName = argv[1];
    const char* instrumentedFuncName = argv[2];
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

    int memAccesses = binaryAnalysis(app, instrumentedFuncName);


    //fprintf(stderr, "Found %d memory accesses\n", memAccesses);
    printInsNum();
    
}