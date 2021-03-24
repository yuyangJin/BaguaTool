#ifndef VERTEX_TYPE_
#define VERTEX_TYPE_

enum VERTEX_TYPE {
  //COMPUTE_NODE = -9,
  //COMBINE = -8,
  CALL_IND_NODE = -7,
  CALL_REC_NODE = -6,
  CALL_NODE = -5,
  FUNC_NODE = -4,
  //COMPOUND = -3,
  BRANCH = -2,
  LOOP_NODE = -1,
  MPI_NODE = 1,
};

#endif