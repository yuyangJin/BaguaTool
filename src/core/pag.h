#ifndef PAG_H
#define PAG_H

#include <stdint.h>
#include <string>
#include "vertex_type.h"

#include "dbg.h"
#include "igraph.h"
#define TRUNK_SIZE 1000

namespace baguatool::core {

struct PAGImpl {
    igraph_t graph;
};

struct PAGVertex {
    igraph_vs_t vertex;
};
}
#endif  // PAG_H