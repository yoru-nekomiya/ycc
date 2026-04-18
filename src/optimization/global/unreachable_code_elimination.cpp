#include "unreachable_code_elimination.hpp"

namespace myLIR::opt {
  static void depth_first_search(const std::shared_ptr<BasicBlock>& bb,
				 std::unordered_set<int>& mark){
    if(!mark.contains(bb->label)){
      mark.insert(bb->label);
      for(const auto& s: bb->succ){
	depth_first_search(s, mark);
      } //for
    } //if
  }
  
  bool unreachable_code_elimination(std::shared_ptr<Function>& fn){
    bool changed = false;

    //1. Traverse basic blocks from the start node by depth first search and mark them.
    std::unordered_set<int> mark = {};
    depth_first_search(fn->start_node, mark);

    //2. Eliminate unmarked blocks.
    for(auto iter = fn->bbs.begin(); iter != fn->bbs.end(); ++iter){
      auto bb = *iter;
      if(!mark.contains(bb->label)){
	//This bb is unreachable

	//Remove the edge bb --> bb's succ
	for(auto it_succ = bb->succ.begin(); it_succ != bb->succ.end(); ++it_succ){
	  auto succ = *it_succ;
	  for(auto it_pred = succ->pred.begin(); it_pred != succ->pred.end(); ++it_pred){
	    auto pred = *it_pred;
	    if(pred->label == bb->label){
	      it_pred = succ->pred.erase(it_pred);
	      --it_pred;
	    } //if
	  } //for it_pred
	} //for it_succ
	
	//Remove the edge bb's pred --> bb
	for(auto it_pred = bb->pred.begin(); it_pred != bb->pred.end(); ++it_pred){
	  auto pred = *it_pred;
	  for(auto it_succ = pred->succ.begin(); it_succ != pred->succ.end(); ++it_succ){
	    auto succ = *it_succ;
	    if(succ->label == bb->label){
	      it_succ = pred->succ.erase(it_succ);
	      --it_succ;
	    } //if
	  } //for it_succ
	} //for it_pred

	//Remove this bb
	iter = fn->bbs.erase(iter);
	--iter;
	changed = true;
      } //if(!mark.contains(bb->label))
    } //for
    return changed;
  }
}
