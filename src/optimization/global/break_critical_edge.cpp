#include "break_critical_edge.hpp"

namespace Lunaria::LIR::Optimizer {
  static void insert_bb_on_edge(BasicBlockPtr& bb,
				BasicBlockPtr& pred,
				BasicBlockPtr& succ,
				FunctionPtr& fn){
    //Insert "bb" on the edge (pred -> succ)
    //reconnect: pred -> bb -> succ
    auto& last_pred = pred->insts.back();
    if(last_pred->opcode == LirKind::LIR_BR){
      if(last_pred->bb1->label == succ->label){
	last_pred->bb1 = bb;
      } else if(last_pred->bb2->label == succ->label){
	last_pred->bb2 = bb;
      }
    } else if(last_pred->opcode == LirKind::LIR_JMP){
      last_pred->bb1 = bb;
    } else {
      assert(false);
    }

    const auto jmp_node = make_node(LirKind::LIR_JMP,
				    nullptr, nullptr, nullptr);
    jmp_node->bbarg = nullptr;
    jmp_node->bb1 = succ;
    bb->insts.push_back(jmp_node);
    
    fn->bbs.push_back(bb);
  }
  
  static void break_critical_edge(FunctionPtr& fn){
    using EdgeVec = std::vector<std::pair<BasicBlockPtr, BasicBlockPtr>>;
    EdgeVec critical_edges;

    //Find critical edges
    for(const auto& bb: fn->bbs){      
      if(bb->succ.size() >= 2){
	for(const auto& succ: bb->succ){
	  if(succ->pred.size() >= 2){
	    //the edge (bb -> succ) is a critical edge
	    critical_edges.push_back(std::make_pair(bb, succ));
	    //std::cerr << "critical_edge: " << bb->label << ", " << succ->label << std::endl;
	  } //if(succ->pred.size() >= 2)
	} //for succ
      } //if(bb->succ.size() >= 2)
    } //for bb

    //Insert an empty block (only containing unconditional jump) on critical edges
    for(auto& [pred, succ]: critical_edges){
      auto new_bb = std::make_shared<BasicBlock>();
      insert_bb_on_edge(new_bb, pred, succ, fn);
    } //for
    
    return;
  }

  void break_critical_edges(ProgramPtr& prog){
    for(auto& fn: prog->fns){
      break_critical_edge(fn);
    }
    constructCFGs(prog);
  }

} //namespace Lunaria::LIR::Optimizer
