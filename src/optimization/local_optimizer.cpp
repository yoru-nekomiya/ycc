#include "opt_utils.hpp"
#include "../ycc.hpp"
#include "../util.hpp"
#include "local/constant_div_reduction.hpp"
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

  static bool convert_optimized_mul(std::shared_ptr<LirNode>& n,
				    int64_t k,
				    std::list<std::shared_ptr<myLIR::LirNode>>::iterator& iter,
				    std::shared_ptr<myLIR::BasicBlock>& bb){
    //convert "n * k" to optimized instruction sequence using lea and shl
    assert(k != 1);
    assert(k != -1);
    assert(k != 0);
    assert(!is_abs_power_of_two(k));
    auto& inst = *iter;

    const bool is_negative = (k < 0);
    const uint64_t abs_k = is_negative ? -k : k;

    //k = odd_k * 2^s
    const int s = std::countr_zero(abs_k);
    const uint64_t odd_k = abs_k >> s;

    std::shared_ptr<LirNode> mad_node = nullptr;
    std::shared_ptr<LirNode> shl_node = nullptr;
    std::shared_ptr<LirNode> add_node = nullptr;
    std::shared_ptr<LirNode> sub_node = nullptr;
    std::shared_ptr<LirNode> s_node = nullptr;
    std::shared_ptr<LirNode> neg_node = nullptr;
    std::shared_ptr<LirNode> mov_node = nullptr;
    int inst_count = 0;
    if(is_abs_power_of_two(odd_k - 1)){
      const int num = std::countr_zero(odd_k - 1);
      //use lea or shl+add
      //x = n * k --> x = mad(n, 2^num, n) n*2^num+n
      if(num <= 3){
	//lea (3,5,9)
	mad_node = make_node(LirKind::LIR_MAD);
	mad_node->d = inst->d;
	mad_node->a = n;
	mad_node->b = n;
	mad_node->scale = (1ULL << num);
	inst_count++;
      } else {
	//shl+add (17,33, ...)
	//x = n * k --> d = n << num; x = d + n;
	shl_node = make_node(LirKind::LIR_SHL);
	shl_node->d = new_reg("");
	shl_node->a = n;
	shl_node->b = make_imm_node(num);

	add_node = make_node(LirKind::LIR_ADD);
	add_node->d = inst->d;
	add_node->a = shl_node->d;
	add_node->b = n;
	inst_count += 2;
      }
    } else if(is_abs_power_of_two(odd_k + 1)){
      //shl+sub (7,15,31, ...)
      //x = n * k --> d = n << num; x = d - n;
      const int num = std::countr_zero(odd_k + 1);
      shl_node = make_node(LirKind::LIR_SHL);
      shl_node->d = new_reg("");
      shl_node->a = n;
      shl_node->b = make_imm_node(num);

      sub_node = make_node(LirKind::LIR_SUB);
      sub_node->d = inst->d;
      sub_node->a = shl_node->d;
      sub_node->b = n;
      inst_count += 2;
    } else {
      //mul
      return false;
    }

    if(s > 0){
      s_node = make_node(LirKind::LIR_SHL);
      s_node->d = inst->d;
      s_node->a = inst->d;
      s_node->b = make_imm_node(s);
      inst_count++;
    }

    if(is_negative){
      neg_node = make_node(LirKind::LIR_SUB);
      neg_node->d = new_reg("");
      neg_node->a = make_imm_node(0);
      neg_node->b = inst->d;
      inst_count++;

      mov_node = make_node(LirKind::LIR_MOV);
      mov_node->d = inst->d;
      mov_node->b = neg_node->d;
    }

    if(inst_count > 3){
      return false;
    }

    int it_count = 0;
    iter = bb->insts.erase(iter);
    if(neg_node){
      iter = bb->insts.insert(iter, mov_node);
      it_count++;
    }
    if(neg_node){
      iter = bb->insts.insert(iter, neg_node);
      it_count++;
    }
    if(s_node){
      iter = bb->insts.insert(iter, s_node);
      it_count++;
    }
    if(sub_node){
      iter = bb->insts.insert(iter, sub_node);
      it_count++;
    }
    if(add_node){
      iter = bb->insts.insert(iter, add_node);
      it_count++;
    }
    if(shl_node){
      iter = bb->insts.insert(iter, shl_node);
      it_count++;
    }
    if(mad_node){
      iter = bb->insts.insert(iter, mad_node);
      it_count++;
    }
    std::advance(iter, it_count-1);
    return true;
  }

  static bool reduce_mul(std::list<std::shared_ptr<myLIR::LirNode>>::iterator& iter,
			 std::shared_ptr<myLIR::BasicBlock>& bb){
    auto inst = *iter;
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
      else if(inst->a->imm == -1){
	//d = -1 * b --> d = 0 - b
	auto sub_node = make_node(LirKind::LIR_SUB);
	sub_node->d = new_reg("");
	sub_node->a = make_imm_node(0);
	sub_node->b = inst->b;
	iter = bb->insts.erase(iter);
	iter = bb->insts.insert(iter, sub_node);
	++iter;

	auto mov_node = make_node(LirKind::LIR_MOV);
	mov_node->d = inst->d;
	mov_node->b = sub_node->d;
	iter = bb->insts.insert(iter, mov_node);	
	changed = true;
      }
      else if(inst->a->imm == 0){
	//d = 0 * b --> d = 0
	/*
	auto xor_node = make_node(LirKind::LIR_BITXOR);
	xor_node->d = inst->d;
	xor_node->a = inst->b;
	xor_node->b = inst->b;
	iter = bb->insts.erase(iter);
	iter = bb->insts.insert(iter, xor_node);
	*/	
	auto imm_node = make_imm_node(0);
	imm_node->d = inst->d;
	iter = bb->insts.erase(iter);
	iter = bb->insts.insert(iter, imm_node);	
	changed = true;
      }
      else if(is_abs_power_of_two(inst->a->imm)){
	convert_mul_to_shift(inst->b, inst->a->imm, iter, bb);
	changed = true;
      }
      else {
	changed = convert_optimized_mul(inst->b, inst->a->imm, iter, bb);
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
      else if(inst->b->imm == -1){
	//d = a * -1 --> d = 0 - a
	auto sub_node = make_node(LirKind::LIR_SUB);
	sub_node->d = new_reg("");
	sub_node->a = make_imm_node(0);
	sub_node->b = inst->a;
	iter = bb->insts.erase(iter);
	iter = bb->insts.insert(iter, sub_node);
	++iter;

	auto mov_node = make_node(LirKind::LIR_MOV);
	mov_node->d = inst->d;
	mov_node->b = sub_node->d;
	iter = bb->insts.insert(iter, mov_node);
	changed = true;
      }
      else if(inst->b->imm == 0){
	//d = a * 0 --> d = 0
	/*
	auto xor_node = make_node(LirKind::LIR_BITXOR);
	xor_node->d = inst->d;
	xor_node->a = inst->a;
	xor_node->b = inst->a;
	iter = bb->insts.erase(iter);
	iter = bb->insts.insert(iter, xor_node);
	*/
	auto imm_node = make_imm_node(0);
	imm_node->d = inst->d;
	iter = bb->insts.erase(iter);
	iter = bb->insts.insert(iter, imm_node);
	changed = true;
      }
      else if(is_abs_power_of_two(inst->b->imm)){
	convert_mul_to_shift(inst->a, inst->b->imm, iter, bb);
	changed = true;
      }
      else {
	changed = convert_optimized_mul(inst->a, inst->b->imm, iter, bb);
      }
    }
    return changed;
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
    for(auto iter_inst = bb->insts.begin(); iter_inst != bb->insts.end(); ++iter_inst){
      auto& inst = *iter_inst;      
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
