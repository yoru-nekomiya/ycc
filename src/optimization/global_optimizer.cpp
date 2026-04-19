#include "opt_utils.hpp"
#include "../ycc.hpp"
#include "../util.hpp"
#include "global/dead_code_elimination.hpp"
#include "global/unreachable_code_elimination.hpp"

namespace myLIR::opt {
  bool optimize_fn(std::shared_ptr<Function>& fn){
    bool changed = false;
    changed = changed || unreachable_code_elimination(fn);
    changed = changed || dead_code_elimination(fn);
    return changed;
  }

  bool merge_basic_block(std::shared_ptr<Function>& fn){
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
	  std::copy(succ->succ.begin(), succ->succ.end(), std::back_inserter(bb->succ));
	  bb->succ.remove(succ);
	  fn->bbs.remove(succ);
	  changed = true;
	} //if(!succ->is_end_node && succ->pred.size() == 1)
      } //if(bb->succ.size() == 1)
    } //for iter

    return changed;
  }
  
} //namespace myLIR::opt
