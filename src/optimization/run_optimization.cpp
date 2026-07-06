#include "opt_utils.hpp"
//#include "../slcc.hpp"
#include "../lunaria.hpp"

namespace Lunaria::LIR::Optimizer {
  void optimize(std::unique_ptr<Program>& prog,
		const std::string& filename,
		bool opt,
		bool emit_cfg){
    bool optimized = false;
    do {
      optimized = false;
      bool local_optimized = false;
      bool global_optimized = false;      
      if(opt){
	for(auto& fn: prog->fns){
	  //local optimization
	  for(auto& bb: fn->bbs){
	    local_optimized = false;
	    do {
	      local_optimized = optimize_bb(bb);
	    } while(local_optimized && (optimized = true));
	  } //for bb	
	} //for fn
      } //if opt        
      
      constructCFGs(prog);
      
      if(opt){
	for(auto& fn: prog->fns){
	  //global optimization	  
	  do {
	    global_optimized = false;
	    global_optimized = global_optimized || merge_basic_block(fn);
	    global_optimized = global_optimized || optimize_fn(fn);
	  } while(global_optimized && (optimized = true));
	  
	} //for fn
      } //if opt
    } while(optimized);
    
    constructCFGs(prog);
    if(emit_cfg) printCFGs(prog, filename);
  }
} //namespace Lunaria::LIR::Optimizer
