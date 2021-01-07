#include <stdio.h>
#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_function.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_point.h"
#include "BPatch_process.h"
#include "BPatch_flowGraph.h"
#include "BPatch_loopTreeNode.h"
#include "BPatch_snippet.h"
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


void traverseLoopTree(BPatch_image* image, BPatch_addressSpace* app, BPatch_flowGraph* fg, BPatch_loopTreeNode * ltNode){
  if (ltNode == NULL) {
		return ;
	}
  vector <BPatch_loopTreeNode *> clist = ltNode->children;
  for (uint i = 0; i < clist.size(); i++) {
    // find inserted function code
    std::vector<BPatch_function*> loop_entry, loop_exit, loop_start, loop_end;
    image->findFunction("code_to_inject_loop_entry", loop_entry);
    image->findFunction("code_to_inject_loop_exit", loop_exit);
    image->findFunction("code_to_inject_loop_start", loop_start);
    image->findFunction("code_to_inject_loop_end", loop_end);
    
    // set inerted functions' args
    std::vector<BPatch_snippet*> openArgs;
    BPatch_snippet* loop_id_arg = new BPatch_constExpr(clist[i]->name());
    openArgs.push_back(loop_id_arg);
    BPatch_funcCallExpr inserted_call_loop_entry(*(loop_entry[0]), openArgs);
    BPatch_funcCallExpr inserted_call_loop_exit(*(loop_exit[0]), openArgs);
    BPatch_funcCallExpr inserted_call_loop_start(*(loop_start[0]), openArgs);
    BPatch_funcCallExpr inserted_call_loop_end(*(loop_end[0]), openArgs);
    
    // find instrumented points of loop
    std::vector<BPatch_point *> *instrumented_points_loop_entry, *instrumented_points_loop_exit, *instrumented_points_loop_start, *instrumented_points_loop_end;
    //image->findFunction(instrumentedFuncName, func);
    instrumented_points_loop_entry = fg->findLoopInstPoints(BPatch_locLoopEntry, clist[i]->loop);
    instrumented_points_loop_exit = fg->findLoopInstPoints(BPatch_locLoopExit, clist[i]->loop);
    instrumented_points_loop_start = fg->findLoopInstPoints(BPatch_locLoopStartIter, clist[i]->loop);
    instrumented_points_loop_end = fg->findLoopInstPoints(BPatch_locLoopEndIter, clist[i]->loop);

    app->insertSnippet(inserted_call_loop_entry, *instrumented_points_loop_entry);
    app->insertSnippet(inserted_call_loop_exit, *instrumented_points_loop_exit);
    app->insertSnippet(inserted_call_loop_start, *instrumented_points_loop_start);
    app->insertSnippet(inserted_call_loop_end, *instrumented_points_loop_end);

    
    //recursive call doLoopTree
    traverseLoopTree(image, app, fg, clist[i]);



    LOG_WARN("%s\n",clist[i]->name())


  }

}



int main(int argc, const char *argv[]) {
    //printf("argc=%d \n",argc);
    if( argc != 3){
      std::cout << "./func_instrument [program] [function]" << std::endl;
      exit(1);
    }
    const char* progName = argv[1];
    //const char* progArgv[] = {argv[1], "-h", NULL};
    //const char* libName = argv[1];
    const char* instrumentedFuncName = argv[2];

    // Use BPatch_* classes to initialize
    BPatch bpatch;
    BPatch_addressSpace *app = bpatch.openBinary(progName, true);
    bool flag = false; 
    flag = app->loadLibrary("liblib.so");
    if (!flag){
      std::cout<<"open shard library liblib.so faild" << std::endl;
    }
    
    BPatch_image* image = app->getImage();
    
    //find code_to_inject_entry function and code_to_inject_exit function
    std::vector<BPatch_function*> func, func_entry, func_exit;
    image->findFunction("code_to_inject_entry", func_entry);
    image->findFunction("code_to_inject_exit", func_exit);

    
    std::cout<<"Find function: "<<func_entry[0]->getName()<<std::endl;
    std::cout<<"Find function: "<<func_exit[0]->getName()<<std::endl;

    
    std::vector<BPatch_snippet*> openArgs;
    BPatch_funcCallExpr instrumented_enter_call_entry(*(func_entry[0]), openArgs);
    BPatch_funcCallExpr instrumented_enter_call_exit(*(func_exit[0]), openArgs);
    
    //func.clear();
    std::vector<BPatch_point *> *instrumented_points_entry, *instrumented_points_exit;
    image->findFunction(instrumentedFuncName, func);

    instrumented_points_entry = func[0]->findPoint(BPatch_entry);
    instrumented_points_exit = func[0]->findPoint(BPatch_exit);

    app->insertSnippet(instrumented_enter_call_entry, *instrumented_points_entry);
    app->insertSnippet(instrumented_enter_call_exit, *instrumented_points_exit);

    BPatch_flowGraph* fg = func[0]->getCFG();
    // traverse the loop (Tarjan) tree
		BPatch_loopTreeNode * ltNode = fg->getLoopTree();
    traverseLoopTree(image, app, fg, ltNode);



    //find print_at_prog_exit to print at the end of program
    func.clear();
    image->findFunction("print_at_prog_exit", func);
    
    std::cout<<"Function is: "<<func[0]->getName()<<std::endl;

    BPatch_funcCallExpr main_enter_call_exit(*(func[0]), openArgs);
    
    func.clear();
    std::vector<BPatch_point *> *main_points_exit;
    image->findFunction("main", func);
    main_points_exit = func[0]->findPoint(BPatch_exit);
    
    app->insertSnippet(main_enter_call_exit, *main_points_exit);

    //rewrite
    BPatch_binaryEdit *appBin = dynamic_cast<BPatch_binaryEdit *>(app);
    
    appBin->writeFile("newbinary");
      
    return 0;
}
