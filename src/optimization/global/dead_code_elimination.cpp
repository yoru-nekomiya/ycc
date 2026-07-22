#include "dead_code_elimination.hpp"


namespace Lunaria::LIR {
  struct LirSharedPtrHash {      
   size_t operator()(const LirNodePtr& p) const { 
     return std::hash<LirNode*>()(p.get());    
   }
  };

  struct BasicBlockSharedPtrHash {      
   size_t operator()(const BasicBlockPtr& p) const { 
     return std::hash<BasicBlock*>()(p.get());    
   }
  };
}

namespace Lunaria::LIR::Optimizer {  
  using PredSet = std::unordered_set<LirNodePtr, LirSharedPtrHash>;
  using PredMap = std::unordered_map<int, PredSet>;
  PredMap bb_to_gen, bb_to_kill, bb_to_in, bb_to_out;

  static PredSet operator+(const PredSet& lhs, const PredSet& rhs){
    //lhs U rhs
    PredSet res = lhs;
    res.insert(rhs.begin(), rhs.end());
    return res;
  }

  static PredSet operator-(const PredSet& lhs, const PredSet& rhs){
    //lhs - rhs
    PredSet res;
    for(const auto& item: lhs){
      if(!rhs.contains(item)) res.insert(item);
    }
    return res;
  }
  
  static void compute_local_predicate(FunctionPtr& fn){
    const auto rev_topo = fn->get_reverse_topological_sort();
    for(const auto& bb: rev_topo){
      PredSet gen, kill;
      for(auto iter = bb->insts.rbegin(); iter != bb->insts.rend(); iter++){
	auto& inst = *iter;
	for(const auto& def: inst->Defs()){
	  gen.erase(def);
	  kill.insert(def);
	}
	for(const auto& use: inst->Uses()){
	  if(!is_imm_int32(use))
	    gen.insert(use);
	}
      } //for iter
      bb_to_gen.insert(std::make_pair(bb->label, gen));
      bb_to_kill.insert(std::make_pair(bb->label, kill));
      bb_to_in.insert(std::make_pair(bb->label, PredSet()));
      bb_to_out.insert(std::make_pair(bb->label, PredSet()));      
    } //for bb
  }

  static void compute_dataflow_equation(FunctionPtr& fn){
    auto worklist = fn->get_reverse_topological_sort();
    std::unordered_set<BasicBlockPtr, BasicBlockSharedPtrHash> workset(worklist.begin(), worklist.end());
    while(!worklist.empty()){
      auto bb = worklist.front();
      worklist.pop_front();
      workset.erase(bb);
      
      const auto in_old = bb_to_in[bb->label];
      const auto gen = bb_to_gen[bb->label];
      const auto kill = bb_to_kill[bb->label];

      //Compute OUT set
      //out[n] = Union(in[s]) (s in succ[n])
      PredSet out;
      if(!bb->is_end_node){
	for(const auto& s: bb->succ){	  
	  out = out + bb_to_in[s->label];
	} //for s
	bb_to_out[bb->label] = out;
      } //if

      //Compute IN set
      //in[n] = gen[n] U (out[n] - kill[n])      
      const auto in = gen + (out - kill);
      bb_to_in[bb->label] = in;
      
      //Compare IN set with the previous IN set      
      if(in_old != in){
	for(const auto& p: bb->pred){
	  if(!workset.contains(p)){
	    worklist.push_back(p);
	    workset.insert(p);
	  }
	}
      }
    } //while
  }

  static bool eliminate_dead_code(FunctionPtr& fn){
    bool changed = false;
    for(auto& bb: fn->bbs){
      auto live = bb_to_out[bb->label];      
      for(auto iter = bb->insts.rbegin(); iter != bb->insts.rend(); iter++){
	auto& inst = *iter;
	const auto defs = inst->Defs();

	//delete inst if it is dead
	if(/*inst->opcode != LirKind::LIR_STORE
	   && inst->opcode != LirKind::LIR_STORE_SPILL
	   && inst->opcode != LirKind::LIR_STORE_ARG
	   && inst->opcode != LirKind::LIR_STORE_STACK
	   && inst->opcode != LirKind::LIR_STORE_LABEL
	   && inst->opcode != LirKind::LIR_RETURN
	   && inst->opcode != LirKind::LIR_BR
	   && inst->opcode != LirKind::LIR_JMP
	   */
	   !defs.empty()
	   && inst->opcode != LirKind::LIR_FUNCALL
	   && !live.contains(inst->d)){
	  iter = std::make_reverse_iterator(bb->insts.erase(std::next(iter).base()));
	  iter--;
	  changed = true;
	  continue;
	}

	for(const auto& def: inst->Defs()){
	  live.erase(def);
	}
	for(const auto& use: inst->Uses()){
	  if(!is_imm_int32(use))
	    live.insert(use);
	}		
      } //for iter
    } //for bb
    return changed;
  }
  
  bool dead_code_elimination(FunctionPtr& fn){
    //1. For all basic blocks, compute local gen and kill sets
    compute_local_predicate(fn);
    
    //2. Compute dataflow equations backwardly
    compute_dataflow_equation(fn);
    
    //3. For all basic blocks, eliminate dead code
    const bool changed = eliminate_dead_code(fn);
    return changed;
  }

} //namespace Lunaria::LIR::Optimizer
