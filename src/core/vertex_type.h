#ifndef VERTEX_TYPE_
#define VERTEX_TYPE_

namespace baguatool::core {

enum VERTEX_TYPE {
  BB_NODE = -12,
  INST_NODE = -11,
  ADDR_NODE = -10,
  // COMPUTE_NODE = -9,
  // COMBINE = -8,
  CALL_IND_NODE = -7,
  CALL_REC_NODE = -6,
  CALL_NODE = -5,
  FUNC_NODE = -4,
  // COMPOUND = -3,
  BRANCH_NODE = -2,
  LOOP_NODE = -1,
  MPI_NODE = 1
};
}

#endif