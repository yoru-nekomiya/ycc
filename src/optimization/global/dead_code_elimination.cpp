#include "dead_code_elimination.hpp"


namespace myLIR {
  struct LirSharedPtrHash {      
   size_t operator()(const std::shared_ptr<LirNode>& p) const { 
     return std::hash<LirNode*>()(p.get());    
   }
  };

  struct BasicBlockSharedPtrHash {      
   size_t operator()(const std::shared_ptr<BasicBlock>& p) const { 
     return std::hash<BasicBlock*>()(p.get());    
   }
  };
}

namespace myLIR::opt {
  static bool is_imm_int32(const std::shared_ptr<LirNode>& n){
    return is_imm(n) && is_int32(n);
  }
  
  using PredSet = std::unordered_set<std::shared_ptr<LirNode>, LirSharedPtrHash>;
  std::unordered_map<int, PredSet> bb_to_gen, bb_to_kill, bb_to_in, bb_to_out;
  
  static void compute_local_predicate(std::shared_ptr<Function>& fn){
    const auto rev_topo = fn->get_reverse_topological_sort();
    for(const auto& bb: rev_topo){
      PredSet gen, kill;
      for(auto iter = bb->insts.rbegin(); iter != bb->insts.rend(); iter++){
	auto& inst = *iter;
	if(inst->opcode == LirKind::LIR_IMM
	   || inst->opcode == LirKind::LIR_LABEL_ADDR){
	  gen.erase(inst->d);
	  kill.insert(inst->d);
	  continue;
	}
	
	if(is_binary_opcode(inst->opcode)){
	  gen.erase(inst->d);
	  kill.insert(inst->d);
	  if(!is_imm_int32(inst->a)) gen.insert(inst->a);
	  if(!is_imm_int32(inst->b)) gen.insert(inst->b);
	  continue;
	} //if is_binary_opcode
	
	if(inst->opcode == LirKind::LIR_MOV
	   || inst->opcode == LirKind::LIR_LOAD){
	  gen.erase(inst->d);
	  kill.insert(inst->d);
	  if(!is_imm_int32(inst->b)) gen.insert(inst->b);
	  continue;
	}
	
	if(inst->opcode == LirKind::LIR_BR){
	  if(!is_imm_int32(inst->b)) gen.insert(inst->b);
	  continue;
	}

	if(inst->opcode == LirKind::LIR_JMP){
	  if(inst->bbarg)
	    if(!is_imm_int32(inst->bbarg)) gen.insert(inst->bbarg);	  
	  continue;
	}
	
	if(inst->opcode == LirKind::LIR_STORE){
	  gen.insert(inst->a);
	  if(!is_imm_int32(inst->b)) gen.insert(inst->b);
	  continue;
	}
	
	if(inst->opcode == LirKind::LIR_STORE_SPILL){
	  gen.insert(inst->a);
	  continue;
	}
	
	if(inst->opcode == LirKind::LIR_RETURN){
	  if(inst->a != nullptr){
	    if(!is_imm_int32(inst->a)) gen.insert(inst->a);
	    continue;
	  }
	}
	
	if(inst->opcode == LirKind::LIR_FUNCALL){
	  gen.erase(inst->d);
	  kill.insert(inst->d);
	  for(int i = 0; i < inst->args.size(); i++){
	    if(!is_imm_int32(inst->args[i])) {
	      gen.insert(inst->args[i]);
	    }
	  }
	  continue;
	}
	
      } //for iter
      bb_to_gen.insert(std::make_pair(bb->label, gen));
      bb_to_kill.insert(std::make_pair(bb->label, kill));
      bb_to_in.insert(std::make_pair(bb->label, PredSet()));
      bb_to_out.insert(std::make_pair(bb->label, PredSet()));      
    } //for bb
  }

  static void compute_dataflow_equation(std::shared_ptr<Function>& fn){
    auto worklist = fn->get_reverse_topological_sort();
    std::unordered_set<std::shared_ptr<BasicBlock>, BasicBlockSharedPtrHash> workset(worklist.begin(), worklist.end());
    while(!worklist.empty()){
      auto bb = worklist.front();
      worklist.pop_front();
      
      auto in_old = bb_to_in[bb->label];
      //auto out_old = bb_to_out[bb->label];
      auto gen = bb_to_gen[bb->label];
      auto kill = bb_to_kill[bb->label];

      //Compute OUT set
      PredSet out;
      if(!bb->is_end_node){
	for(const auto& s: bb->succ){
	  const auto in_succ = bb_to_in[s->label];	  
	  for(const auto& item: in_succ) out.insert(item);
	} //for s
	bb_to_out[bb->label] = out;
      } //if

      //Compute IN set
      auto in = out;
      for(const auto& item: kill) in.erase(item);
      for(const auto& item: gen) in.insert(item);
      bb_to_in[bb->label] = in;

      //Compare IN set with the previous IN set
      bool changed = false;      
      if(in_old != in) changed = true;
      if(changed){
	for(const auto& p: bb->pred){
	  if(!workset.contains(p)){
	    worklist.push_back(p);
	    workset.insert(p);
	  }
	}
      }
    } //while
  }

  static bool eliminate_dead_code(std::shared_ptr<Function>& fn){
    bool changed = false;
    for(auto& bb: fn->bbs){
      auto live = bb_to_out[bb->label];      
      for(auto iter = bb->insts.rbegin(); iter != bb->insts.rend(); iter++){
	auto& inst = *iter;

	//delete inst if it is dead
	if(inst->opcode != LirKind::LIR_STORE
	   && inst->opcode != LirKind::LIR_STORE_SPILL
	   && inst->opcode != LirKind::LIR_STORE_ARG
	   && inst->opcode != LirKind::LIR_RETURN
	   && inst->opcode != LirKind::LIR_BR
	   && inst->opcode != LirKind::LIR_JMP
	   && inst->opcode != LirKind::LIR_FUNCALL
	   && inst->opcode != LirKind::LIR_CAST
	   && !live.contains(inst->d)){
	  iter = std::make_reverse_iterator(bb->insts.erase(std::next(iter).base()));
	  iter--;
	  changed = true;
	  continue;
	}
	
	if(inst->opcode == LirKind::LIR_IMM
	   || inst->opcode == LirKind::LIR_LABEL_ADDR){
	  live.erase(inst->d);
	  continue;
	}
	
	if(is_binary_opcode(inst->opcode)){
	  live.erase(inst->d);
	  if(!is_imm_int32(inst->a)) live.insert(inst->a);
	  if(!is_imm_int32(inst->b)) live.insert(inst->b);
	  continue;
	} //if is_binary_opcode
	
	if(inst->opcode == LirKind::LIR_MOV
	   || inst->opcode == LirKind::LIR_LOAD){
	  live.erase(inst->d);
	  if(!is_imm_int32(inst->b)) live.insert(inst->b);
	  continue;
	}
	
	if(inst->opcode == LirKind::LIR_BR){
	  if(!is_imm_int32(inst->b)) live.insert(inst->b);
	  continue;
	}

	if(inst->opcode == LirKind::LIR_JMP){
	  if(inst->bbarg)
	    if(!is_imm_int32(inst->bbarg)) live.insert(inst->bbarg);	  
	  continue;
	}
	
	if(inst->opcode == LirKind::LIR_STORE){
	  live.insert(inst->a);
	  if(!is_imm_int32(inst->b)) live.insert(inst->b);
	  continue;
	}
	
	if(inst->opcode == LirKind::LIR_STORE_SPILL){
	  live.insert(inst->a);
	  continue;
	}
	
	if(inst->opcode == LirKind::LIR_RETURN){
	  if(inst->a != nullptr){
	    if(!is_imm_int32(inst->a)) live.insert(inst->a);
	    continue;
	  }
	}
	
	if(inst->opcode == LirKind::LIR_FUNCALL){
	  live.erase(inst->d);
	  for(int i = 0; i < inst->args.size(); i++){
	    if(!is_imm_int32(inst->args[i])) {
	      live.insert(inst->args[i]);
	    }
	  }
	  continue;
	}
      } //for iter
    } //for bb
    return changed;
  }
  
  bool dead_code_elimination(std::shared_ptr<Function>& fn){
    //1. For all basic blocks, compute local gen and kill sets.
    compute_local_predicate(fn);
    
    //2. Compute dataflow equations backwardly.
    compute_dataflow_equation(fn);
    
    //3. For all basic blocks, eliminate dead code
    const bool changed = eliminate_dead_code(fn);
    return changed;
  }

} //namespace myLIR::opt
