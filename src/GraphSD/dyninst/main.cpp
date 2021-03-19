#include "static_analysis.h"

int main(int argc, char *argv[]){


	StaticAnalysis* static_analysis = new StaticAnalysis(argv[1]);

  static_analysis->CaptureProgramCallGraph();

  static_analysis->ExtractFuncStructureGraph();

  static_analysis->DumpAllFunctionGraph();

	
// #ifdef DEBUG_COUT
//   cout << "==============================" <<endl;
//   printFunc2Node();
//   cout << "==============================" <<endl;
// #endif

//   Node* root = func_2_node["main"];
//   expand(root);

// #ifdef DEBUG_COUT
//   printTree(root,0);
//   cout << "==============================" <<endl;
// #endif

//   trim(root,0);

// #ifdef DEBUG_COUT
//   printTree(root,0);
//   cout << "==============================" <<endl;
// #endif

//   generateID(root, 0);
// 	generateChildID(root);

//   // Write tree to a file
//   string outputFilename = string(argv[1]) + ".psg";
// 	ofstream fout(outputFilename.c_str());

// 	if (!fout.good()) {
// 		cout << "Failed to open output file\n";
// 		return true;
// 	}

// #ifdef DEBUG_COUT
//   printTree(root,0);
// #endif

//   recordAllTrees(root, fout); // first print root

//   fout.close();
//   //cout << "==============================" <<endl;
}