#include "opt_utils.hpp"
//#include "../slcc.hpp"
#include "../lunaria.hpp"
#include "../util.hpp"
#include "global/dead_code_elimination.hpp"
#include "global/unreachable_code_elimination.hpp"
#include "global/lazy_code_motion.hpp"

namespace Lunaria::LIR::Optimizer {
  bool optimize_fn(FunctionPtr& fn){
    bool changed = false;
    changed = changed || unreachable_code_elimination(fn);
    changed = changed || dead_code_elimination(fn);
    changed = changed || lazy_code_motion(fn);
    return changed;
  }

  bool merge_basic_block(FunctionPtr& fn){
    //This function merges bb1 and bb2, where bb1 is the unique predecessor of bb2, and bb2 is the unique successor of bb1.
    //Thus, this function eliminates the unconditional jump instruction of bb1, and the edge between bb1 and bb2.
    bool changed = false;
    for(auto iter = fn->bbs.begin(); iter != fn->bbs.end(); ++iter){
      auto bb = *iter;
      if(bb->succ.size() == 1){
	auto succ = *(bb->succ.begin());
	if(!succ->is_end_node && succ->pred.size() == 1){
	  //Merge bb and succ
	  //Delete bb's jmp
	  auto it_last = bb->insts.end();
	  --it_last;
	  assert((*it_last)->opcode == LirKind::LIR_JMP);
	  if((*it_last)->bbarg){
	    auto mov_node = make_node(LirKind::LIR_MOV,
				      (*it_last)->bb1->param,
				      nullptr,
				      (*it_last)->bbarg);
	    it_last = bb->insts.insert(it_last, mov_node);
	    ++it_last;
	  }
	  it_last = bb->insts.erase(it_last);

	  //Copy succ's instructions into bb
	  for(const auto& i: succ->insts){
	    bb->insts.insert(bb->insts.end(), i);
	  }

	  //Change edges	  
	  bb->succ = succ->succ;
	  for(auto& succ_succ: succ->succ){	    
	    for(auto it_pred = succ_succ->pred.begin(); it_pred != succ_succ->pred.end(); ++it_pred){
	      auto p = *it_pred;
	      if(p->label == succ->label){
		it_pred = succ_succ->pred.erase(it_pred);
		--it_pred;
	      }
	    } //for it_pred
	    succ_succ->pred.insert(succ_succ->pred.end(), bb);
	  } //for succ_succ
	  succ->pred.clear();
	  succ->succ.clear();
	  changed = true;
	} //if(!succ->is_end_node && succ->pred.size() == 1)
      } //if(bb->succ.size() == 1)
    } //for iter

    return changed;
  }
  
} //namespace Lunaria::LIR::Optimizer
