#ifndef DEAD_CODE_ELIMINATION_HPP
#define DEAD_CODE_ELIMINATION_HPP

#include "../../ycc.hpp"
#include "../opt_utils.hpp"
#include "../../util.hpp"

namespace Lunaria::LIR::Optimizer {
  bool dead_code_elimination(std::shared_ptr<Function>& fn);
}

#endif
