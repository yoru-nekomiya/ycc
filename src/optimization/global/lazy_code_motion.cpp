#include "lazy_code_motion.hpp"

namespace Lunaria::LIR::Optimizer {
  using InstIterator = std::list<LirNodePtr>::iterator;
  
  struct InstructionLocation {
    BasicBlockPtr bb; //The basic block to which this expression belongs
    InstIterator it; //Iterator in the instruction list of "bb"
  };

  struct ExpressionInfo {
    int expr_id;
    LirNodePtr sample_inst;
    
    // プログラム全体で、この式IDが出現する具体的な位置リスト
    std::vector<InstructionLocation> occurrences;
  };  
  
  static void compute_local_predicate(const FunctionPtr& fn){
    //Compute TRANSP, AVLOC, ANTLOC for each bb
    const auto rev_topo = fn->get_reverse_topological_sort();
    for(const auto& bb: rev_topo){

    } //for bb
  }
  
  bool lazy_code_motion(FunctionPtr& fn){    
    
    //compute_local_predicate(fn);    
    //compute_dataflow_equation(fn);
    //const bool changed = code_motion(fn);
    //return changed;
    return false;
  }
  
} //namespace Lunaria::LIR::Optimizer
