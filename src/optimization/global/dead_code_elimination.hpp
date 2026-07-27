#ifndef DEAD_CODE_ELIMINATION_HPP
#define DEAD_CODE_ELIMINATION_HPP

#include "../../lunaria.hpp"
#include "../opt_utils.hpp"
#include "../../util.hpp"

namespace Lunaria::LIR::Optimizer {
  bool dead_code_elimination(FunctionPtr& fn);
}

#endif
