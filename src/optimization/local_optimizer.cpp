#include "opt_utils.hpp"
#include "../ycc.hpp"
#include "../util.hpp"

namespace myLIR {
  struct LirSharedPtrHash {
    size_t operator()(const std::shared_ptr<LirNode>& p) const {
      return std::hash<LirNode*>()(p.get());
    }
  };
}

namespace myLIR::opt {
  static bool is_binary_opcode(LirKind k){
    return k == LirKind::LIR_ADD
      || k == LirKind::LIR_SUB
      || k == LirKind::LIR_MUL
      || k == LirKind::LIR_DIV
      || k == LirKind::LIR_REM
      || k == LirKind::LIR_EQ
      || k == LirKind::LIR_NE
      || k == LirKind::LIR_LT
      || k == LirKind::LIR_LE
      || k == LirKind::LIR_PTR_ADD
      || k == LirKind::LIR_PTR_SUB
      || k == LirKind::LIR_PTR_DIFF
      || k == LirKind::LIR_SHL
      || k == LirKind::LIR_SHR
      || k == LirKind::LIR_SAR
      || k == LirKind::LIR_BITOR
      || k == LirKind::LIR_BITAND
      || k == LirKind::LIR_BITXOR;
  }
  
  static bool constant_propagation(std::shared_ptr<BasicBlock>& bb){
    bool changed = false;
    std::unordered_map<std::shared_ptr<LirNode>, int64_t, LirSharedPtrHash> table;

    for(auto inst: bb->insts){
      //generation
      if(is_imm(inst)){
	table.insert(std::make_pair(inst->d, inst->imm));
      }

      //kill
      if(inst->opcode == LirKind::LIR_CAST){
	table.erase(inst->a);
      }

      //replace
      if(is_binary_opcode(inst->opcode)
	 && inst->opcode != LirKind::LIR_DIV
	 && inst->opcode != LirKind::LIR_REM){
	if(table.contains(inst->a) && !is_imm(inst->a)){
	  inst->a->opcode = LirKind::LIR_IMM;
	  inst->a->imm = table.find(inst->a)->second;
	  changed = true;
	}
	if(table.contains(inst->b) && !is_imm(inst->b)){
	  inst->b->opcode = LirKind::LIR_IMM;
	  inst->b->imm = table.find(inst->b)->second;
	  changed = true;
	}
      } //if is_binary_opcode
    } //for inst
    return changed;
  }
  
  static bool peephole(std::shared_ptr<BasicBlock>& bb){
    bool changed = false;
    for(auto iter_inst = bb->insts.begin(); iter_inst != bb->insts.end();++iter_inst){
      auto& inst = *iter_inst;
      if(is_binary_opcode(inst->opcode)
	 && is_imm(inst->a)
	 && is_imm(inst->b)){
	//constant folding
	if(inst->opcode == LirKind::LIR_ADD){
	  const int64_t c = inst->a->imm + inst->b->imm;
	  auto imm_node = std::make_shared<LirNode>();
	  imm_node->opcode = LirKind::LIR_IMM;
	  imm_node->d = inst->d;
	  imm_node->imm = c;

	  //delete original instruction
	  iter_inst = bb->insts.erase(iter_inst);

	  //insert MOV instruction
	  iter_inst = bb->insts.insert(iter_inst, imm_node);	  
	  changed = true;
	  continue;
	}
      }
    } //for inst
    return changed;
  }
  
  bool optimize_bb(std::shared_ptr<BasicBlock>& bb){
    //changed IR --> return true
    //otherwise --> return false
    bool changed = false;
    changed = changed || peephole(bb);
    changed = changed || constant_propagation(bb);
    return changed;
  }
}
