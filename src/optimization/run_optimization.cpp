#include "opt_utils.hpp"
#include "../ycc.hpp"

namespace myLIR::opt {
  void optimize(std::unique_ptr<Program>& prog,
		const std::string& filename){

    for(auto& fn: prog->fns){
      for(auto& bb: fn->bbs){
	bool optimized = optimize_bb(bb);
	while(optimized){
	  optimized = optimize_bb(bb);
	}
      }
    }
    
    constructCFGs(prog);
    printCFGs(prog, filename);
  }
} //namespace myLIR::opt
