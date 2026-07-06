#ifndef UNREACHABLE_CODE_ELIMINATION_HPP
#define UNREACHABLE_CODE_ELIMINATION_HPP

//#include "../../slcc.hpp"
#include "../../lunaria.hpp"
#include "../opt_utils.hpp"
#include "../../util.hpp"

namespace Lunaria::LIR::Optimizer {
  bool unreachable_code_elimination(std::shared_ptr<Function>& fn);
}

#endif
