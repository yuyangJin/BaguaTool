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

#include <CFG.h>
#include <CodeObject.h>
#include <InstructionDecoder.h>
#include <LineInformation.h>
#include <Symtab.h>

#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "baguatool.h"
#include "core/pag.h"
#include "static_analysis.h"

using namespace Dyninst;
using namespace ParseAPI;
// using namespace SymtabAPI;
using namespace InstructionAPI;

namespace baguatool::graph_sd {

StaticAnalysis::StaticAnalysis(char *binary_name) { this->sa = std::make_unique<StaticAnalysisImpl>(binary_name); }

StaticAnalysis::~StaticAnalysis() {}

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

// Comparison functions to sort blocks, edges, loops for more
// deterministic output.

// FIXME: It is better to use compartision function as lambda funcion instead of a traditional function
//        if these functions won't be used in many diffrent cases

// Sort Blocks by start address, low to high.
//static bool BlockLessThan(Block *b1, Block *b2) { return b1->start() < b2->start(); }

// Returns: the min entry VMA for the loop, or else 0 if the loop is
// somehow invalid.  Irreducible loops have more than one entry
// address.
/*
static VMA LoopMinEntryAddr(Loop *loop) {
  if (loop == nullptr) {
    return 0;
  }

  std::vector<Block *> entBlocks;
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
*/

// Sort Loops (from their LoopTreeNodes) by min entry VMAs.
/*
static bool LoopTreeLessThan(LoopTreeNode *n1, LoopTreeNode *n2) {
  return LoopMinEntryAddr(n1->loop) < LoopMinEntryAddr(n2->loop);
}
*/

// Capture a Program Call Graph (PCG)
void StaticAnalysisImpl::CaptureProgramCallGraph() {
  // core::ProgramAbstractionGraph* pcg = new
  // core::ProgramAbstractionGraph("program_call_graph");

  // Get function list
  const CodeObject::funclist &func_list = this->co->funcs();

  // Traverse through all functions
  for (auto func: func_list) {
    this->addr_2_func_name[func->addr()] = func->name();
    const Function::edgelist &elist = func->callEdges();

    // Traverse through all fuunction calls in this function
    for (const auto& e: elist) {
      VMA src = e->src()->last();
      VMA targ = e->trg()->start();
      this->call_graph_map[src] = targ;
    }
  }
}

// Capture function call structure in this function but not in the loop
void StaticAnalysisImpl::ExtractCallStructure(core::ProgramAbstractionGraph *func_struct_graph, std::vector<Block *> &bvec,
                                              int parent_id) {

  // Traverse through all blocks
  for (auto b: bvec) {
    // If block is visited, it means it is inside the loop
    if (!this->visited_block_map[b]) {
      this->visited_block_map[b] = true;

      // Traverse through all instructions
      for (auto inst: b->targets()) {
        // Only analyze CALL type instruction
        if (inst->type() == CALL) {
#ifdef DEBUG_COUT
          std::cout << "Call : "
               << decoder.decode((unsigned char *)func->isrc()->getPtrToInstruction((*it)->src()->start()))
                      .format()
#endif
          Address entry_addr = inst->src()->last();
          Address exit_addr = inst->src()->last();
          std::string call_name = this->addr_2_func_name[this->call_graph_map[entry_addr]];
          int call_vertex_id = 0;

          // Add a CALL vertex, including MPI_CALL, INDIRECT_CALL, and CALL
          auto startsWith = [](const std::string& s, const std::string& sub) -> bool {
            return s.find(sub) == 0;
          };
          if (startsWith(call_name, "MPI_") || startsWith(call_name, "_MPI_") || startsWith(call_name, "mpi_") ||
              startsWith(call_name, "_mpi_")) {  // MPI communication calls
            call_vertex_id = func_struct_graph->AddVertex();
            func_struct_graph->SetVertexBasicInfo(call_vertex_id, core::MPI_NODE, call_name.c_str());
            // call_vertex_id = new Node(core::MPI_NODE, entry_addr, exit_addr);
          } else if (call_name.empty()) {  // Function calls that are not
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
          std::cout << "Call : " << std::call_name << " addr : " << std::hex << entry_addr << " - " << exit_addr << std::dec << endl;
#endif
        }
      }
    }
  }
}

void StaticAnalysisImpl::ExtractLoopStructure(core::ProgramAbstractionGraph *func_struct_graph, LoopTreeNode *loop_tree,
                                              int depth, int parent_id) {
  if (loop_tree == nullptr) {
    return;
  }

  // process the children of the loop tree
  std::vector<LoopTreeNode *> child_loop_list = loop_tree->children;
  std::unordered_map<Loop*, VMA> loop_min_entry_addr;
  for (auto loop_tree_node: child_loop_list) {
    auto loop = loop_tree_node->loop;
    VMA addr = 0;
    if (loop != nullptr) {
      std::vector<Block *> ent_blocks;
      int num_ents = loop->getLoopEntries(ent_blocks);
      if (num_ents >= 1) {
        addr = VMA_MAX;
        for (int i = 0; i < num_ents; ++i) {
          addr = std::min(addr, ent_blocks[i]->start());
        }
      }
    }

    loop_min_entry_addr[loop] = addr;
  }

  std::sort(child_loop_list.begin(), child_loop_list.end(), [&loop_min_entry_addr](LoopTreeNode *a, LoopTreeNode *b) -> bool {
    return loop_min_entry_addr[a->loop] < loop_min_entry_addr[b->loop];
  });

  for (auto loop_tree_node: child_loop_list) {
    std::vector<Block *> blocks;
    loop_tree_node->loop->getLoopBasicBlocks(blocks);

    std::sort(blocks.begin(), blocks.end(), [](Block *a, Block *b) -> bool {
      return a->start() < b->start();
    });

    Address entry_addr = blocks.front()->start();
    Address exit_addr = blocks.back()->end();

    std::string loop_name = loop_tree_node->name();

    int loop_vertex_id = func_struct_graph->AddVertex();
    func_struct_graph->SetVertexBasicInfo(loop_vertex_id, core::LOOP_NODE, loop_name.c_str());
    func_struct_graph->SetVertexDebugInfo(loop_vertex_id, entry_addr - 8, exit_addr - 8);

    func_struct_graph->AddEdge(parent_id, loop_vertex_id);

#ifdef DEBUG_COUT
    for (int i = 0; i < depth; i++) cout << "  ";
    std::cout << "Loop : " << std::loop_name << " addr : " << std::hex << entry_addr << " - " << exit_addr << std::dec << std::endl;
#endif

    this->ExtractLoopStructure(func_struct_graph, loop_tree_node, depth + 1, loop_vertex_id);
    if (loop_tree_node->numCallees() > 0) {
      this->ExtractCallStructure(func_struct_graph, blocks, loop_vertex_id);
    }
  }
}

// Extract structure graph for each fucntion
void StaticAnalysisImpl::IntraProceduralAnalysis() {
  // Traverse through all functions
  for (auto func: this->co->funcs()) {
    Address entry_addr = func->addr();
    std::string func_name = func->name();

    // Create a graph for each function
    auto func_struct_graph = new core::ProgramAbstractionGraph();
    func_struct_graph->GraphInit(func_name.c_str());
    this->func_2_graph[func_name] = func_struct_graph;


    const ParseAPI::Function::blocklist &blist = func->blocks();
    std::vector<Block *> bvec(blist.begin(), blist.end());
    std::sort(bvec.begin(), bvec.end(), [](Block *a, Block *b) -> bool {
      return a->start() < b->start();
    });

    entry_addr = bvec.front()->start();
    Address exit_addr = bvec.back()->last();

    // Create root vertex in the graph
    int func_vertex_id = func_struct_graph->AddVertex();
    func_struct_graph->SetVertexBasicInfo(func_vertex_id, core::FUNC_NODE, func_name.c_str());
    func_struct_graph->SetVertexDebugInfo(func_vertex_id, entry_addr, exit_addr);

// Add DebugInfo attributes
// auto func_vertex_id = new Node(core::FUNC_NODE, entry_addr, exit_addr);

#ifdef DEBUG_COUT
    std::cout << "Function : " << func_name << " addr : " << hex << entry_addr << "/" << entry_addr << " - " << exit_addr
         << dec << std::endl;
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
  std::string dir_name = std::string(getcwd(NULL, 0)) + std::string("/") + std::string(this->binary_name) + std::string(".pag");

  printf("%s\n", dir_name.c_str());
  // TODO: this syscall needs to be wrapped
  if (access(dir_name.c_str(), F_OK)) {
    if (mkdir(dir_name.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) {
      printf("mkdir failed\n");
      return;
    }
  }

  // Traverse through all functions
  for (auto func: this->co->funcs()) {
    std::string func_name = func->name();
    core::ProgramAbstractionGraph *func_struct_graph = this->func_2_graph[func_name];
    std::stringstream ss;
    ss << "./" << this->binary_name << ".pag/" << func_name << ".gml";
    auto file_name = ss.str();
    this->DumpFunctionGraph(func_struct_graph, file_name.c_str());
  }
}

void StaticAnalysisImpl::GetBinaryName() {
  UNIMPLEMENTED();
  //   return this->binary_name;
}

// int CallerCallee(core::ProgramAbstractionGraph* func_struct_graph, const int
// vertex_id){
//   if (func_struct_graph->GetVertexType(vertex_id) == CALL){

//   }
// }

void StaticAnalysisImpl::InterProceduralAnalysis() { UNIMPLEMENTED(); }

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
}  // namespace baguatool::graph_sd