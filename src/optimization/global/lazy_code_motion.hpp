#ifndef LAZY_CODE_MOTION_HPP
#define LAZY_CODE_MOTION_HPP

#include "../../lunaria.hpp"
#include "../opt_utils.hpp"
#include "../../util.hpp"

namespace Lunaria::LIR::Optimizer {
  bool lazy_code_motion(FunctionPtr& fn);
  void break_critical_edges(ProgramPtr& prog);
}

#endif
