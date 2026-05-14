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
      || k == LirKind::LIR_STORE_STACK
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
	table.insert_or_assign(inst->d, inst->imm);
      }
      if(inst->opcode == LirKind::LIR_MOV
	 && is_imm(inst->b)){
	table.insert_or_assign(inst->d, inst->b->imm);
      }
      //-----------------------------

      //replace----------------------
      //cast
      if(inst->opcode == LirKind::LIR_CAST){
	//table.erase(inst->d);
	if(table.contains(inst->b) && !is_imm(inst->b)){
	  inst->b->opcode = LirKind::LIR_IMM;
	  inst->b->imm = table.find(inst->b)->second;
	  changed = true;
	}
      }

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

  static bool copy_propagation(std::shared_ptr<BasicBlock>& bb){
    bool changed = false;
    std::unordered_map<std::shared_ptr<LirNode>, std::shared_ptr<LirNode>, LirSharedPtrHash> table; //d --> b

    for(auto inst: bb->insts){
      //generation and kill-------------------
      if(inst->opcode == LirKind::LIR_MOV
	 && !is_imm(inst->b)){
	table.insert_or_assign(inst->d, inst->b);
      }
      //-----------------------------

      //d=a+b
      //replace a and b if they are registered in table,
      //and kill d in table
      //binary opcode----------
      if(is_binary_opcode(inst->opcode)){
	if(table.contains(inst->a) && !is_imm(inst->a)){
	  inst->a = table.find(inst->a)->second;
	  changed = true;
	}
	if(table.contains(inst->b) && !is_imm(inst->b)){
	  inst->b = table.find(inst->b)->second;
	  changed = true;
	}	
	table.erase(inst->d);
      } //if is_binary_opcode
      //----------

      //unary opcode----------
      if(inst->opcode == LirKind::LIR_STORE){
	if(table.contains(inst->a)){
	  inst->a = table.find(inst->a)->second;
	  changed = true;
	}
	if(table.contains(inst->b) && !is_imm(inst->b)){
	  inst->b = table.find(inst->b)->second;
	  changed = true;
	}	
      }

      if(inst->opcode == LirKind::LIR_LOAD
	 || inst->opcode == LirKind::LIR_CAST){
	if(table.contains(inst->b)){
	  inst->b = table.find(inst->b)->second;
	  changed = true;
	}
	table.erase(inst->d);
      }

      if(inst->opcode == LirKind::LIR_RETURN){
	if(inst->a != nullptr
	   && !is_imm(inst->a)
	   && table.contains(inst->a)){
	  inst->a = table.find(inst->a)->second;
	  changed = true;
	}
      }

      if(inst->opcode == LirKind::LIR_BR
	 || inst->opcode == LirKind::LIR_STORE_STACK){
	if(table.contains(inst->b) && !is_imm(inst->b)){
	  inst->b = table.find(inst->b)->second;
	  changed = true;
	}
      }
      
      if(inst->opcode == LirKind::LIR_JMP
	 && inst->bbarg != nullptr
	 && !is_imm(inst->bbarg)
	 && table.contains(inst->bbarg)){
	inst->bbarg = table.find(inst->bbarg)->second;
	changed = true;
      }
      
      //----------

      //function call
      if(inst->opcode == LirKind::LIR_FUNCALL){
	for(int i = 0; i < inst->args.size(); ++i){
	  if(!is_imm(inst->args[i])
	     && table.contains(inst->args[i])){
	    inst->args[i] = table.find(inst->args[i])->second;
	    changed = true;
	  }
	}
      }
      
    } //for inst
    
    return changed;
  }

  int64_t calc_constant(const std::shared_ptr<LirNode>& inst){
    int64_t c = 0;
    switch(inst->opcode){
    case LirKind::LIR_ADD:
      c = inst->a->imm + inst->b->imm; break;
    case LirKind::LIR_SUB:
      c = inst->a->imm - inst->b->imm; break;
    case LirKind::LIR_MUL:
      c = inst->a->imm * inst->b->imm; break;
    case LirKind::LIR_MULHIGH:{
      const __int128 res = (__int128)inst->a->imm * inst->b->imm;
      c = (int64_t)(res >> 64);
      break;
    }
    case LirKind::LIR_MAD:
      c = inst->a->imm + inst->b->imm * inst->scale; break;
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
    default:
      std::cerr << "unknown opcode (constant_folding)\n"; exit(1);
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
	  iter = bb->insts.erase(iter);
	  iter = bb->insts.insert(iter, add_node);
	  changed = true;
	  continue;
	}
      } //if LIR_SHL

      if(inst->opcode == LirKind::LIR_BR
	 && is_imm(inst->b)){
	//branch folding
	auto bb = (inst->b->imm == 0) ? inst->bb2 : inst->bb1;
	auto jmp_node = make_node(LirKind::LIR_JMP,
				  nullptr,
				  nullptr,
				  nullptr);
	jmp_node->bb1 = bb;
	jmp_node->bbarg = nullptr;
	iter = bb->insts.erase(iter);
	iter = bb->insts.insert(iter, jmp_node);
	changed = true;
	continue;
      } //if LIR_BR      
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

  static bool
  propagate_stack_address(std::shared_ptr<BasicBlock>& bb){
    //v1 <- [rbp-4]
    //v2 <- [v1]
    //-->
    //v2 <- [rbp-4]

    bool changed = false;
    std::unordered_map<std::shared_ptr<LirNode>, std::shared_ptr<Lunaria::Var>, LirSharedPtrHash> st_offset_table; //vn --> lvar
    for(auto iter = bb->insts.begin(); iter != bb->insts.end(); ++iter){
      auto inst = *iter;
      if(inst->opcode == LirKind::LIR_LVAR){
	st_offset_table.insert_or_assign(inst->d, inst->lvar);
      }

      if(inst->opcode == LirKind::LIR_LOAD){
	if(st_offset_table.contains(inst->b)){
	  auto node = make_node(LirKind::LIR_LOAD_STACK,
				inst->d,
				nullptr,
				nullptr);
	  node->lvar = st_offset_table.find(inst->b)->second;
	  node->type_size = inst->type_size;
	  iter = bb->insts.erase(iter);
	  iter = bb->insts.insert(iter, node);
	  changed = true;
	}
      }

      if(inst->opcode == LirKind::LIR_STORE){
	if(st_offset_table.contains(inst->a)){
	  auto node = make_node(LirKind::LIR_STORE_STACK,
				nullptr,
				nullptr,
				inst->b);
	  node->lvar = st_offset_table.find(inst->a)->second;
	  node->type_size = inst->type_size;
	  iter = bb->insts.erase(iter);
	  iter = bb->insts.insert(iter, node);
	  changed = true;
	}
      }
    }
    return changed;
  }

  /*
  static bool
  redundant_load_elimination(std::shared_ptr<BasicBlock>& bb){
    //redundant load
    //v1 <- [rbp-4]
    //v2 <- [v1]
    //v3 <- [v1] ==> v3 <- v2

    //store-to-load forwarding
    //[v4] <- v5
    //v6 <- [v4] ==> v6 <- v5 (or Cast: v7(64bit) <- v5(32bit); v6 <- v7; if store to v4 is 32bit)

    bool changed = false;
    std::unordered_map<std::shared_ptr<LirNode>, int, LirSharedPtrHash> st_offset_table; //vn --> stack_offset
    std::unordered_map<std::shared_ptr<LirNode>, std::shared_ptr<LirNode>, LirSharedPtrHash> value_table; //[vn] --> vn (Load)
    std::unordered_map<std::shared_ptr<LirNode>, std::shared_ptr<LirNode>, LirSharedPtrHash> store_table; //[vn] --> inst (Store)
    for(auto iter = bb->insts.begin(); iter != bb->insts.end(); ++iter){
      auto inst = *iter;
      if(inst->opcode == LirKind::LIR_LVAR){
	st_offset_table.insert_or_assign(inst->d, inst->lvar->offset);
      }

      if(inst->opcode == LirKind::LIR_LOAD){
	if(value_table.contains(inst->b)){
	  auto mov_node = make_node(LirKind::LIR_MOV,
				    inst->d,
				    nullptr,
				    value_table.find(inst->b)->second);
	  iter = bb->insts.erase(iter);
	  iter = bb->insts.insert(iter, mov_node);
	  changed = true;
	}
	else if(store_table.contains(inst->b)){
	  
	  auto store_inst = store_table.find(inst->b)->second;
	  std::shared_ptr<LirNode> node = make_node(LirKind::LIR_CAST,
						    new_reg("", 8),
						    nullptr,
						    store_inst->b);
	  node->type_size = store_inst->type_size;
	  auto mov_node = make_node(LirKind::LIR_MOV,
				    inst->d,
				    nullptr,
				    node->d);
	  
	  iter = bb->insts.erase(iter);
	  iter = bb->insts.insert(iter, node);
	  ++iter;
	  iter = bb->insts.insert(iter, mov_node);
	  changed = true;
	  
	}
	else {
	  value_table.insert(std::make_pair(inst->b, inst->d));
	}
      }

      if(inst->opcode == LirKind::LIR_STORE){
	
	if(st_offset_table.contains(inst->a)){
	  store_table.insert_or_assign(inst->a, inst);
	  value_table.erase(inst->a);
	} else {
	  value_table.clear();
	  store_table.clear();
	  store_table.insert(std::make_pair(inst->a, inst));
	}	
      }
      
      if(inst->opcode == LirKind::LIR_FUNCALL){
	value_table.clear();
	store_table.clear();
      }
    } //for iter
    
    return changed;
  }
  */

  static bool
  redundant_load_elimination(std::shared_ptr<BasicBlock>& bb){
    //redundant load
    //Load_stack: v1 <- [rbp-4]
    //Load_stack: v2 <- [rbp-4] ==> v2 <- v1

    //store-to-load forwarding
    //[rbp-8] <- v3
    //v4 <- [rbp-8] ==> v4 <- v3 (or Cast: v5(64bit) <- v3(32bit); v4 <- v5; if store to rbp-8 is 32bit)

    bool changed = false;
    std::unordered_map<int, std::shared_ptr<LirNode>> st_offset_table; //stack_offset --> vn (Load)
    std::unordered_map<int, std::shared_ptr<LirNode>> store_table; //stack_offset --> inst (Store)
    for(auto iter = bb->insts.begin(); iter != bb->insts.end(); ++iter){
      auto inst = *iter;
      if(inst->opcode == LirKind::LIR_LOAD_STACK){
	if(st_offset_table.contains(inst->lvar->offset)){
	  auto mov_node = make_node(LirKind::LIR_MOV,
				    inst->d,
				    nullptr,
				    st_offset_table.find(inst->lvar->offset)->second);
	  iter = bb->insts.erase(iter);
	  iter = bb->insts.insert(iter, mov_node);
	  changed = true;
	}
	else if(store_table.contains(inst->lvar->offset)){
	  
	  auto store_inst = store_table.find(inst->lvar->offset)->second;
	  std::shared_ptr<LirNode> node = make_node(LirKind::LIR_CAST,
						    new_reg("", 8),
						    nullptr,
						    store_inst->b);
	  node->type_size = store_inst->type_size;
	  auto mov_node = make_node(LirKind::LIR_MOV,
				    inst->d,
				    nullptr,
				    node->d);
	  
	  iter = bb->insts.erase(iter);
	  iter = bb->insts.insert(iter, node);
	  ++iter;
	  iter = bb->insts.insert(iter, mov_node);
	  changed = true;
	  
	}
	else {
	  st_offset_table.insert(std::make_pair(inst->lvar->offset, inst->d));
	}
      }

      if(inst->opcode == LirKind::LIR_STORE_STACK){	
	  store_table.insert_or_assign(inst->lvar->offset, inst);
	  st_offset_table.erase(inst->lvar->offset);
      }
      
      if(inst->opcode == LirKind::LIR_FUNCALL
	 || inst->opcode == LirKind::LIR_STORE){
	st_offset_table.clear();
	store_table.clear();
      }
    } //for iter
    
    return changed;
  }
  
  bool optimize_bb(std::shared_ptr<BasicBlock>& bb){
    //changed IR --> return true
    //otherwise --> return false
    bool changed = false;
    bool cp = false;
    bool cf = false;
    bool copy_prop = false;
    do {
      cp = constant_propagation(bb);
      cf = constant_foldings(bb);
      copy_prop = copy_propagation(bb);
      changed = changed || cp || cf || copy_prop;
    } while(cp || cf);
    changed = changed || peephole(bb);
    //changed = changed || eliminate_redundant_load_from_stack(bb);
    changed = changed || redundant_load_elimination(bb);
    changed = changed || propagate_stack_address(bb);
    return changed;
  }
}
