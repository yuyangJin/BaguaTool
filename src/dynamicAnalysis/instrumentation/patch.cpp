#include <stdio.h>
#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_function.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_point.h"

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
    
    std::cout<<"Function is: "<<func_entry[0]->getName()<<std::endl;
    std::cout<<"Function is: "<<func_exit[0]->getName()<<std::endl;
    
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
