#ifndef GRAPH_COLORING_REG_ALLOC_HPP
#define GRAPH_COLORING_REG_ALLOC_HPP

#include "../ycc.hpp"
#include "../util.hpp"
#include "opt_utils.hpp"
#include <iterator>
#include <utility>

namespace myRegAlloc {
  void graph_coloring_register_allocation_x86_64(std::unique_ptr<myLIR::Program>& prog);
  
}

#endif
