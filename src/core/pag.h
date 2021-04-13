#ifndef PAG_H
#define PAG_H

#include <stdint.h>

#include <string>

#include "common/common.h"
#include "igraph.h"
#include "vertex_type.h"
#define TRUNK_SIZE 1000

namespace baguatool::core {

struct PAG_graph_t {
  igraph_t graph;
};

struct PAG_vertex_set_t {
  igraph_vs_t vertices;
};

struct PAG_vertex_t {
  igraph_integer_t vertex_id;
};

}  // namespace baguatool::core
#endif  // PAG_H