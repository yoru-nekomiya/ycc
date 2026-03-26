#include "opt_utils.hpp"
#include "../ycc.hpp"
#include "../util.hpp"
#include "global/dead_code_elimination.hpp"

namespace myLIR::opt {
  bool optimize_fn(std::shared_ptr<Function>& fn){
    bool changed = false;
    changed = changed || dead_code_elimination(fn);
    return changed;
  }
  
} //namespace myLIR::opt
