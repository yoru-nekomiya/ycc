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

  static bool is_unary_opcode(LirKind k){
    return k == LirKind::LIR_MOV
      || k == LirKind::LIR_STORE
      //|| k == LirKind::LIR_LOAD
      //|| k == LirKind::LIR_RETURN
      || k == LirKind::LIR_BR;
  }
  
  static bool constant_propagation(std::shared_ptr<BasicBlock>& bb){
    bool changed = false;
    std::unordered_map<std::shared_ptr<LirNode>, int64_t, LirSharedPtrHash> table;

    for(auto inst: bb->insts){
      //generation-------------------
      if(is_imm(inst)){
	table.insert(std::make_pair(inst->d, inst->imm));
      }
      //-----------------------------

      //kill-------------------------
      if(inst->opcode == LirKind::LIR_CAST){
	table.erase(inst->a);
      }
      //-----------------------------

      //replace----------------------
      //binary opcode
      if(is_binary_opcode(inst->opcode)){
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

      //unary opcode
      if(is_unary_opcode(inst->opcode)){
	if(table.contains(inst->b) && !is_imm(inst->b)){
	  inst->b->opcode = LirKind::LIR_IMM;
	  inst->b->imm = table.find(inst->b)->second;
	  changed = true;
	}
      } //if is_unary_opcode

      //LIR_RETURN opcode
      if(inst->opcode == LirKind::LIR_RETURN){
	if(table.contains(inst->a) && !is_imm(inst->a)){
	  inst->a->opcode = LirKind::LIR_IMM;
	  inst->a->imm = table.find(inst->a)->second;
	  changed = true;
	}
      }

      //LIR_JMP opcode
      if(inst->opcode == LirKind::LIR_JMP){
	if(inst->bbarg){
	  if(table.contains(inst->bbarg) && !is_imm(inst->bbarg)){
	    inst->bbarg->opcode = LirKind::LIR_IMM;
	    inst->bbarg->imm = table.find(inst->bbarg)->second;
	    changed = true;
	  }
	}
      }

      //LIR_FUNCALL opcode
      if(inst->opcode == LirKind::LIR_FUNCALL){
	for(int i = 0; i < inst->args.size(); i++){
	  if(table.contains(inst->args[i]) && !is_imm(inst->args[i])){
	    inst->args[i]->opcode = LirKind::LIR_IMM;
	    inst->args[i]->imm = table.find(inst->args[i])->second;
	    changed = true;
	  }
	}
      }
    } //for inst
    return changed;
  }

  int64_t calc_constant(LirKind k, const std::shared_ptr<LirNode>& inst){
    int c;
    switch(k){
    case LirKind::LIR_ADD:
      c = inst->a->imm + inst->b->imm; break;
    case LirKind::LIR_SUB:
      c = inst->a->imm - inst->b->imm; break;
    case LirKind::LIR_MUL:
      c = inst->a->imm * inst->b->imm; break;
    case LirKind::LIR_DIV:
      c = inst->a->imm / inst->b->imm; break;
    case LirKind::LIR_REM:
      c = inst->a->imm % inst->b->imm; break;
    case LirKind::LIR_EQ:
      c = inst->a->imm == inst->b->imm; break;
    case LirKind::LIR_NE:
      c = inst->a->imm != inst->b->imm; break;
    case LirKind::LIR_LT:
      c = inst->a->imm < inst->b->imm; break;
    case LirKind::LIR_LE:
      c = inst->a->imm <= inst->b->imm; break;
    case LirKind::LIR_SHL:
      c = inst->a->imm << inst->b->imm; break;
    case LirKind::LIR_SHR:
      c = inst->a->imm >> inst->b->imm; break;
    case LirKind::LIR_SAR:
      c = inst->a->imm >> inst->b->imm; break;
    case LirKind::LIR_BITOR:
      c = inst->a->imm | inst->b->imm; break;
    case LirKind::LIR_BITAND:
      c = inst->a->imm & inst->b->imm; break;
    case LirKind::LIR_BITXOR:
      c = inst->a->imm ^ inst->b->imm; break;
    }
    return c;
  }
  
  static bool peephole(std::shared_ptr<BasicBlock>& bb){
    bool changed = false;
    for(auto iter_inst = bb->insts.begin(); iter_inst != bb->insts.end(); ++iter_inst){
      auto& inst = *iter_inst;
      if(is_binary_opcode(inst->opcode)
	 && is_imm(inst->a)
	 && is_imm(inst->b)){
	//constant folding	
	const int64_t c = calc_constant(inst->opcode, inst);
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
