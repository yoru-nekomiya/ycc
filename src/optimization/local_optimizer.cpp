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
    assert(n != 0);
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

  static bool reduce_mul(std::list<std::shared_ptr<myLIR::LirNode>>::iterator& iter,
			 std::shared_ptr<myLIR::BasicBlock>& bb){
    auto& inst = *iter;
    bool changed = false;
    if(is_imm(inst->a) && !is_imm(inst->b)){
      if(inst->a->imm == 1){
	//d = 1 * b --> d = b
	auto mov_node = make_node(LirKind::LIR_MOV);
	mov_node->d = inst->d;
	mov_node->b = inst->b;
	iter = bb->insts.erase(iter);
	iter = bb->insts.insert(iter, mov_node);
	changed = true;
      }
      else if(is_abs_power_of_two(inst->a->imm)){
	convert_mul_to_shift(inst->b, inst->a->imm, iter, bb);
	changed = true;
      }
    }
    else if(!is_imm(inst->a) && is_imm(inst->b)){
      if(inst->b->imm == 1){
	//d = a * 1 --> d = a
	auto mov_node = make_node(LirKind::LIR_MOV);
	mov_node->d = inst->d;
	mov_node->b = inst->a;
	iter = bb->insts.erase(iter);
	iter = bb->insts.insert(iter, mov_node);
	changed = true;
      }
      else if(is_abs_power_of_two(inst->b->imm)){
	convert_mul_to_shift(inst->a, inst->b->imm, iter, bb);
	changed = true;
      }
    }
    return changed;
  }

  static void convert_div_and_rem_to_shift(std::shared_ptr<LirNode>& x,
					   int64_t c,
					   std::list<std::shared_ptr<myLIR::LirNode>>::iterator& iter,
					   std::shared_ptr<myLIR::BasicBlock>& bb){
    //d = x / c
    //-->
    //k = log2(|c|)
    //temp = x >> 63 (SAR)
    //bias = temp >>> (64-k) (SHR)
    //x2 = x + bias
    //d = x2 >> k (SAR)

    //d = x % c
    //-->
    //k = log2(|c|)
    //temp = x >> 63 (SAR)
    //bias = temp >>> (64-k) (SHR)
    //x2 = x + bias
    //mask = |c| - 1
    //x3 = x2 & mask
    //d = x3 - bias
    const LirKind kind = (*iter)->opcode;
    const int k = get_log2(c);
    const int mask = std::abs(c) - 1;
    
    auto sar_node_1 = make_node(LirKind::LIR_SAR);
    sar_node_1->d = new_reg("");
    sar_node_1->a = x;
    sar_node_1->b = make_imm_node(63);

    auto shr_node = make_node(LirKind::LIR_SHR);
    shr_node->d = new_reg("");
    shr_node->a = sar_node_1->d;
    shr_node->b = make_imm_node(64-k);

    auto add_node = make_node(LirKind::LIR_ADD);
    add_node->d = new_reg("");
    add_node->a = x;
    add_node->b = shr_node->d;

    if(kind == LirKind::LIR_DIV){
      auto sar_node_2 = make_node(LirKind::LIR_SAR);
      sar_node_2->a = add_node->d;
      sar_node_2->b = make_imm_node(k);
      
      if(c >= 0){
	sar_node_2->d = (*iter)->d;
	
	iter = bb->insts.erase(iter);
	iter = bb->insts.insert(iter, sar_node_2);
	iter = bb->insts.insert(iter, add_node);
	iter = bb->insts.insert(iter, shr_node);
	iter = bb->insts.insert(iter, sar_node_1);
	std::advance(iter, 3);
      } else {
	//d2 = x2 >> k (SAR)
	//d = 0-d2 
	sar_node_2->d = new_reg("");
	
	auto sub_node = make_node(LirKind::LIR_SUB);
	sub_node->d = (*iter)->d;
	sub_node->a = make_imm_node(0);
	sub_node->b = sar_node_2->d;
	
	iter = bb->insts.erase(iter);
	iter = bb->insts.insert(iter, sub_node);
	iter = bb->insts.insert(iter, sar_node_2);
	iter = bb->insts.insert(iter, add_node);
	iter = bb->insts.insert(iter, shr_node);
	iter = bb->insts.insert(iter, sar_node_1);
	std::advance(iter, 4);
      }
    }    
    else if(kind == LirKind::LIR_REM){
      auto and_node = make_node(LirKind::LIR_BITAND);
      and_node->d = new_reg("");
      and_node->a = add_node->d;
      and_node->b = make_imm_node(mask);

      auto sub_node = make_node(LirKind::LIR_SUB);
      sub_node->d = (*iter)->d;
      sub_node->a = and_node->d;
      sub_node->b = shr_node->d;

      iter = bb->insts.erase(iter);
      iter = bb->insts.insert(iter, sub_node);
      iter = bb->insts.insert(iter, and_node);
      iter = bb->insts.insert(iter, add_node);
      iter = bb->insts.insert(iter, shr_node);
      iter = bb->insts.insert(iter, sar_node_1);
      std::advance(iter, 4);
    }    
  }
  
  static bool reduce_div_and_rem(std::list<std::shared_ptr<myLIR::LirNode>>::iterator& iter,
				 std::shared_ptr<myLIR::BasicBlock>& bb){
    auto& inst = *iter;
    bool changed = false;
    if(!is_imm(inst->a) && is_imm(inst->b)){
      if(inst->b->imm == 1 || inst->b->imm == -1){
	if(inst->opcode == LirKind::LIR_DIV){
	  //d = x / 1  --> d = x
	  //d = x / -1 --> d = 0-x
	  if(inst->b->imm == 1){
	    auto mov_node = make_node(LirKind::LIR_MOV);
	    mov_node->d = inst->d;
	    mov_node->b = inst->a;
	    iter = bb->insts.erase(iter);
	    iter = bb->insts.insert(iter, mov_node);
	  } else if(inst->b->imm == -1){
	    auto sub_node = make_node(LirKind::LIR_SUB);
	    sub_node->d = inst->d;
	    sub_node->a = make_imm_node(0);
	    sub_node->b = inst->a;
	    iter = bb->insts.erase(iter);
	    iter = bb->insts.insert(iter, sub_node);
	  }
	} else if(inst->opcode == LirKind::LIR_REM){
	  //d = x % 1   --> d = 0
	  //d = x % -1  --> d = 0
	  auto imm_node = make_imm_node(0);
	  imm_node->d = inst->d;
	  iter = bb->insts.erase(iter);
	  iter = bb->insts.insert(iter, imm_node);
	}
	changed = true;
      } 
      else if(is_abs_power_of_two(inst->b->imm)){
	convert_div_and_rem_to_shift(inst->a, inst->b->imm, iter, bb);
	changed = true;
      }
    }
    return changed;
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
	const bool c = reduce_mul(iter_inst, bb);
	changed = changed || c;
	if(c) continue;
      } //if LIR_MUL

      if(inst->opcode == LirKind::LIR_DIV
	 || inst->opcode == LirKind::LIR_REM){
	const bool c = reduce_div_and_rem(iter_inst, bb);
	changed = changed || c;
	if(c) continue;
      } //if LIR_DIV
      
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
	  auto mov_node = make_node(LirKind::LIR_MOV);
	  mov_node->d = inst->d;
	  mov_node->b = table.find(inst->lvar->offset)->second;
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

  static bool dead_code_elimination(std::shared_ptr<BasicBlock>& bb){
    bool changed = false;
    std::unordered_set<std::shared_ptr<LirNode>, LirSharedPtrHash> live;
    for(auto iter = bb->insts.rbegin(); iter != bb->insts.rend(); ++iter){
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
	if(!is_imm(inst->a)) live.insert(inst->a);
	if(!is_imm(inst->b)) live.insert(inst->b);
	continue;
      } //if is_binary_opcode

      if(inst->opcode == LirKind::LIR_MOV
	 || inst->opcode == LirKind::LIR_LOAD){
	live.erase(inst->d);
	if(!is_imm(inst->b)) live.insert(inst->b);
	continue;
      }

      if(inst->opcode == LirKind::LIR_BR){
	if(!is_imm(inst->b)) live.insert(inst->b);
	continue;
      }

      if(inst->opcode == LirKind::LIR_STORE){
	live.insert(inst->a);
	if(!is_imm(inst->b)) live.insert(inst->b);
	continue;
      }

      if(inst->opcode == LirKind::LIR_STORE_SPILL){
	live.insert(inst->a);
	continue;
      }

      if(inst->opcode == LirKind::LIR_RETURN){
	if(inst->a != nullptr){
	  if(!is_imm(inst->a)) live.insert(inst->a);
	  continue;
	}
      }

      if(inst->opcode == LirKind::LIR_FUNCALL){
	live.erase(inst->d);
	for(int i = 0; i < inst->args.size(); i++){
	  if(!is_imm(inst->args[i])) {
	    live.insert(inst->args[i]);
	  }
	}
	continue;
      }
      
    } //for iter
    return changed;
  }
  
  bool optimize_bb(std::shared_ptr<BasicBlock>& bb){
    //changed IR --> return true
    //otherwise --> return false
    bool changed = false;
    changed = changed || peephole(bb);
    changed = changed || constant_propagation(bb);
    changed = changed || eliminate_redundant_load_from_stack(bb);
    return changed;
  }
}
