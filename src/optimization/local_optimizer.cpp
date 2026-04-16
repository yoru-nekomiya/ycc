#include "opt_utils.hpp"
#include "../ycc.hpp"
#include "../util.hpp"
#include "local/constant_div_reduction.hpp"
#include "local/constant_mul_reduction.hpp"
#include <bit>

namespace myLIR {
  struct LirSharedPtrHash {
    size_t operator()(const std::shared_ptr<LirNode>& p) const {
      return std::hash<LirNode*>()(p.get());
    }
  };
}

namespace myLIR::opt {
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

  int64_t calc_constant(const std::shared_ptr<LirNode>& inst){
    int64_t c;
    switch(inst->opcode){
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
      c = static_cast<int64_t>(static_cast<uint64_t>(inst->a->imm) >> inst->b->imm); break;
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

  static void constant_folding(std::list<std::shared_ptr<myLIR::LirNode>>::iterator& iter,
			       std::shared_ptr<myLIR::BasicBlock>& bb){
    auto& inst = *iter;
    const int64_t c = calc_constant(inst);
    auto imm_node = make_imm_node(c);
    imm_node->d = inst->d;
    
    iter = bb->insts.erase(iter);
    iter = bb->insts.insert(iter, imm_node);	  
  }

  static bool constant_foldings(std::shared_ptr<BasicBlock>& bb){
    bool changed = false;
    for(auto iter_inst = bb->insts.begin(); iter_inst != bb->insts.end(); ++iter_inst){
      auto& inst = *iter_inst;
      if(is_binary_opcode(inst->opcode)
	 && is_imm(inst->a)
	 && is_imm(inst->b)){
	constant_folding(iter_inst, bb);	
	changed = true;
	continue;	
      } //if
    }
    return changed;
  }
  
  static bool peephole(std::shared_ptr<BasicBlock>& bb){
    bool changed = false;
    for(auto iter = bb->insts.begin(); iter != bb->insts.end(); ++iter){
      auto inst = *iter;
      if(inst->opcode == LirKind::LIR_ADD){
	if(is_imm(inst->a)
	   && inst->a->imm == 0
	   && !is_imm(inst->b)){
	  //d=0+b --> d=b
	  auto mov_node = make_node(LirKind::LIR_MOV,
				    inst->d,
				    nullptr,
				    inst->b);
	  /*
	  mov_node->d = inst->d;
	  mov_node->b = inst->b;
	  */
	  iter = bb->insts.erase(iter);
	  iter = bb->insts.insert(iter, mov_node);
	  changed = true;
	  continue;
	}
	else if(is_imm(inst->b)
		&& inst->b->imm == 0
		&& !is_imm(inst->a)){
	  //d=a+0 --> d=a
	  auto mov_node = make_node(LirKind::LIR_MOV,
				    inst->d,
				    nullptr,
				    inst->a);
	  /*
	  mov_node->d = inst->d;
	  mov_node->b = inst->a;
	  */
	  iter = bb->insts.erase(iter);
	  iter = bb->insts.insert(iter, mov_node);
	  changed = true;
	  continue;
	}
      } //if LIR_ADD

      if(inst->opcode == LirKind::LIR_SUB){
	if(is_imm(inst->b)
	   && inst->b->imm == 0
	   && !is_imm(inst->a)){
	  //d=a-0 --> d=a
	  auto mov_node = make_node(LirKind::LIR_MOV,
				    inst->d,
				    nullptr,
				    inst->a);
	  /*
	  mov_node->d = inst->d;
	  mov_node->b = inst->a;
	  */
	  iter = bb->insts.erase(iter);
	  iter = bb->insts.insert(iter, mov_node);
	  changed = true;
	  continue;
	}
      } //if LIR_SUB
      
      if(inst->opcode == LirKind::LIR_MUL){
	const bool c = reduce_mul(iter, bb);
	changed = changed || c;
	if(c) continue;
      } //if LIR_MUL

      if(inst->opcode == LirKind::LIR_DIV
	 || inst->opcode == LirKind::LIR_REM){
	const bool c = reduce_div_and_rem(iter, bb);
	changed = changed || c;
	if(c) continue;
      } //if LIR_DIV || LIR_REM

      if(inst->opcode == LirKind::LIR_SHL){
	if(is_imm(inst->b)
	   && inst->b->imm == 1
	   && !is_imm(inst->a)){
	  //d=a<<1 --> d=a+a
	  auto add_node = make_node(LirKind::LIR_ADD,
				    inst->d,
				    inst->a,
				    inst->a);
	  /*
	  add_node->d = inst->d;
	  add_node->a = inst->a;
	  add_node->b = inst->a;
	  */
	  iter = bb->insts.erase(iter);
	  iter = bb->insts.insert(iter, add_node);
	  changed = true;
	  continue;
	}
      } //if LIR_SHL
      
    } //for inst
    return changed;
  }

  static bool
  eliminate_redundant_load_from_stack(std::shared_ptr<BasicBlock>& bb){
    //Lunaria assumes that rbp keeps the same value during the execution of a function.
    //Load_lvar: v11 <- [rbp-8]
    //Load_lvar: v12 <- [rbp-8]
    //-->
    //Load_lvar: v11 <- [rbp-8]
    //v12 <- v11
    bool changed = false;
    std::unordered_map<int, std::shared_ptr<LirNode>> table;
    for(auto iter = bb->insts.begin(); iter != bb->insts.end(); ++iter){
      auto& inst = *iter;
      if(inst->opcode == LirKind::LIR_LVAR){
	if(table.contains(inst->lvar->offset)){
	  auto mov_node = make_node(LirKind::LIR_MOV,
				    inst->d,
				    nullptr,
				    table.find(inst->lvar->offset)->second);
	  /*
	  mov_node->d = inst->d;
	  mov_node->b = table.find(inst->lvar->offset)->second;
	  */
	  iter = bb->insts.erase(iter);
	  iter = bb->insts.insert(iter, mov_node);
	  changed = true;
	} else {
	  //register offset
	  table.insert(std::make_pair(inst->lvar->offset, inst->d));
	}
      } //if LIR_LVAR
    } //for
    return changed;
  }
  
  bool optimize_bb(std::shared_ptr<BasicBlock>& bb){
    //changed IR --> return true
    //otherwise --> return false
    bool changed = false;
    bool cp = false;
    bool cf = false;
    do {
      cp = constant_propagation(bb);
      cf = constant_foldings(bb);      
      changed = changed || cp || cf;
    } while(cp || cf);
    changed = changed || peephole(bb);
    changed = changed || eliminate_redundant_load_from_stack(bb);
    return changed;
  }
}
