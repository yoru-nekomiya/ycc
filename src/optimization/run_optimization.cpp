#include "opt_utils.hpp"
#include "../ycc.hpp"

namespace myLIR::opt {
  void optimize(std::unique_ptr<Program>& prog,
		const std::string& filename){
    constructCFGs(prog);
    printCFGs(prog, filename);
  }
} //namespace myLIR::opt
