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

  int64_t calc_constant(const std::shared_ptr<LirNode>& inst){
    int c;
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

  static void constant_folding(std::list<std::shared_ptr<myLIR::LirNode>>::iterator& iter,
			       std::shared_ptr<myLIR::BasicBlock>& bb){
    auto& inst = *iter;
    const int64_t c = calc_constant(inst);
    auto imm_node = std::make_shared<LirNode>();
    imm_node->opcode = LirKind::LIR_IMM;
    imm_node->d = inst->d;
    imm_node->imm = c;
    
    //delete original instruction
    iter = bb->insts.erase(iter);
    
    //insert MOV instruction
    iter = bb->insts.insert(iter, imm_node);	  
  }

  template <std::integral T>
  static bool is_abs_power_of_two(T n) {
    // 0 は2の冪乗ではない
    if(n == 0) return false;
    
    // 負の最小値 (INT_MINなど) の絶対値は、正の最大値 (INT_MAX) を超えるため
    // 安全に扱うために符号なし型 (std::make_unsigned_t<T>) にキャストする
    using U = std::make_unsigned_t<T>;
    
    U abs_n;
    if(n < 0){
      // n が負の場合、2の補数表現での絶対値を計算
      // (単に -n とすると INT_MIN でオーバーフローする可能性があるためキャストが先)
      abs_n = static_cast<U>(0) - static_cast<U>(n);
    } else {
      abs_n = static_cast<U>(n);
    }
    
    // C++20: 立っているビットが1つだけなら true
    return std::has_single_bit(abs_n);
  }

  template <std::integral T>
  static int get_log2(T n) {
    using U = std::make_unsigned_t<T>;
    return std::countr_zero(static_cast<U>(n));
  }

  static void convert_mul_to_shift(std::shared_ptr<LirNode>& node,
				   int64_t c,
				   std::list<std::shared_ptr<myLIR::LirNode>>::iterator& iter,
				   std::shared_ptr<myLIR::BasicBlock>& bb){
    const int num_shift = get_log2(c);
    auto imm_node = std::make_shared<LirNode>();
    imm_node->opcode = LirKind::LIR_IMM;
    imm_node->imm = num_shift;

    auto shift_node = std::make_shared<LirNode>();
    shift_node->opcode = LirKind::LIR_SHL;
    shift_node->a = node;      
    shift_node->b = imm_node;
      
    if(c >= 0){
      //d <- a * c
      //-->
      //d <- a << log2(c)      
      shift_node->d = (*iter)->d;      

      iter = bb->insts.erase(iter);
      iter = bb->insts.insert(iter, shift_node);
    } else {
      //d <- a * c
      //-->
      //d2 <- a << log2(c)
      //d <- 0 - d2
      shift_node->d = new_reg("");
      auto zero_node = std::make_shared<LirNode>();
      zero_node->opcode = LirKind::LIR_IMM;
      zero_node->imm = 0;
      
      auto sub_node = std::make_shared<LirNode>();
      sub_node->opcode = LirKind::LIR_SUB;
      sub_node->d = (*iter)->d;
      sub_node->a = zero_node;
      sub_node->b = shift_node->d;

      iter = bb->insts.erase(iter);
      iter = bb->insts.insert(iter, sub_node);
      iter = bb->insts.insert(iter, shift_node);      
      iter++;
    }
  }
  
  static bool peephole(std::shared_ptr<BasicBlock>& bb){
    bool changed = false;
    for(auto iter_inst = bb->insts.begin(); iter_inst != bb->insts.end(); ++iter_inst){
      auto& inst = *iter_inst;
      
      if(is_binary_opcode(inst->opcode)
	 && is_imm(inst->a)
	 && is_imm(inst->b)){
	//constant folding
	constant_folding(iter_inst, bb);	
	changed = true;
	continue;	
      } //if is_binary_opcode
      
      if(inst->opcode == LirKind::LIR_MUL){
	if(is_imm(inst->a) && !is_imm(inst->b)){
	  if(is_abs_power_of_two(inst->a->imm)){
	    convert_mul_to_shift(inst->b, inst->a->imm, iter_inst, bb);
	    changed = true;
	    continue;
	  }
	}
	if(!is_imm(inst->a) && is_imm(inst->b)){
	  if(is_abs_power_of_two(inst->b->imm)){
	    convert_mul_to_shift(inst->a, inst->b->imm, iter_inst, bb);
	    changed = true;
	    continue;
	  }
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
