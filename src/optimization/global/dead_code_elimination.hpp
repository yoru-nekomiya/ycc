#ifndef DEAD_CODE_ELIMINATION_HPP
#define DEAD_CODE_ELIMINATION_HPP

#include "../../ycc.hpp"
#include "../opt_utils.hpp"
#include "../../util.hpp"

namespace myLIR::opt {
  bool dead_code_elimination(std::shared_ptr<Function>& fn);
}

#endif
