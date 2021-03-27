#ifndef STATIC_ANALYSIS_H_
#define STATIC_ANALYSIS_H_

#include <CFG.h>
#include <CodeObject.h>
#include <InstructionDecoder.h>
#include <LineInformation.h>
#include <Symtab.h>
#include <map>
#include <unordered_map>
#include "core/pag.h"

#include "baguatool.h"

#include "dbg.h"

using namespace std;
using namespace Dyninst;
using namespace ParseAPI;
// using namespace SymtabAPI;
using namespace InstructionAPI;
typedef unsigned long int VMA;
typedef long int VMASigned;  // useful for offsets

//#define DEBUG_COUT

#ifndef MAX_LOOP_DEPTH
#define MAX_LOOP_DEPTH 2
#endif

#define VMA_MAX (~((unsigned long int)(0)))
#define PTR_TO_BFDVMA(x) ((unsigned long int)(uintptr_t)(x))
#define BFDVMA_TO_PTR(x, totype) ((totype)(uintptr_t)(x))

#define PTR_TO_VMA(x) PTR_TO_BFDVMA(x)
#define VMA_TO_PTR(x, totype) BFDVMA_TO_PTR(x, totype)

namespace baguatool::graph_sd {

class StaticAnalysisImpl {
   private:
    SymtabCodeSource *sts;
    CodeObject *co;
    unordered_map<Block *, bool> visited_block_map;
    unordered_map<Address, string> addr_2_func_name;
    map<VMA, VMA> call_graph_map;
    unordered_map<string, core::ProgramAbstractionGraph *> func_2_graph;
    char *binary_name;

   public:
    StaticAnalysisImpl(char *binary_name) {
        // Create a new binary code object from the filename argument
        this->sts = new SymtabCodeSource(binary_name);
        this->co = new CodeObject(this->sts);

        // Parse the binary
        this->co->parse();

        this->binary_name = binary_name;
    }

    ~StaticAnalysisImpl() {}

    void IntraProceduralAnalysis();
    void ExtractLoopStructure(core::ProgramAbstractionGraph *func_struct_graph, LoopTreeNode *loop_tree, int depth,
                              int parent_id);
    void ExtractCallStructure(core::ProgramAbstractionGraph *func_struct_graph, vector<Block *> &bvec, int parent_id);
    void InterProceduralAnalysis();
    void CaptureProgramCallGraph();
    void DumpFunctionGraph(core::ProgramAbstractionGraph *func_struct_graph, const char *file_name);
    void DumpAllFunctionGraph();
    void GetBinaryName();
};
}

#endif