#ifndef BREAK_CRITICAL_EDGE_HPP
#define BREAK_CRITICAL_EDGE_HPP

#include "../../lunaria.hpp"
#include "../opt_utils.hpp"

namespace Lunaria::LIR::Optimizer {
  void break_critical_edges(ProgramPtr& prog);
}

#endif
