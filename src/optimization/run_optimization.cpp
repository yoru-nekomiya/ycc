#include "opt_utils.hpp"
#include "../ycc.hpp"

namespace myLIR::opt {
  void optimize(std::unique_ptr<Program>& prog,
		const std::string& filename,
		bool opt,
		bool emit_cfg){    
    if(opt){
      for(auto& fn: prog->fns){
	//local optimization
	for(auto& bb: fn->bbs){
	  bool optimized = false;
	  do {
	    optimized = optimize_bb(bb);
	  } while(optimized);
	} //for bb	
      } //for fn
    } //if opt        

    constructCFGs(prog);
    
    if(opt){
      for(auto& fn: prog->fns){
	//global optimization
	bool optimized = false;
	do {
	  optimized = false;
	  optimized = optimized || merge_basic_block(fn);
	  optimized = optimized || optimize_fn(fn);
	} while(optimized);
	
      } //for fn
    } //if opt

    constructCFGs(prog);
    if(emit_cfg) printCFGs(prog, filename);
  }
} //namespace myLIR::opt
