#ifndef UNREACHABLE_CODE_ELIMINATION_HPP
#define UNREACHABLE_CODE_ELIMINATION_HPP

#include "../../ycc.hpp"
#include "../opt_utils.hpp"
#include "../../util.hpp"

namespace myLIR::opt {
  bool unreachable_code_elimination(std::shared_ptr<Function>& fn);
}

#endif
