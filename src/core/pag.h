#ifndef PAG_H
#define PAG_H

#include <stdint.h>

#include <string>

#include "common/common.h"
#include "igraph.h"
#include "vertex_type.h"
#define TRUNK_SIZE 1000

namespace baguatool::core {

struct PAGImpl {
  igraph_t graph;
};

struct PAGVertex {
  igraph_vs_t vertex;
};
}  // namespace baguatool::core
#endif  // PAG_H