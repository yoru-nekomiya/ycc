#include "opt_utils.hpp"
//#include "../slcc.hpp"
#include "../lunaria.hpp"
#include "../util.hpp"
#include "local/constant_div_reduction.hpp"
#include "local/constant_mul_reduction.hpp"
#include <bit>

namespace Lunaria::LIR {
  struct LirSharedPtrHash {
    size_t operator()(const LirNodePtr& p) const {
      return std::hash<LirNode*>()(p.get());
    }
  };
}

namespace Lunaria::LIR::Optimizer {
  /*
  static bool is_unary_opcode(LirKind k){
    return k == LirKind::LIR_MOV
      || k == LirKind::LIR_STORE
      || k == LirKind::LIR_STORE_STACK
      || k == LirKind::LIR_STORE_LABEL
      //|| k == LirKind::LIR_LOAD
      //|| k == LirKind::LIR_RETURN
      || k == LirKind::LIR_BR;
  }
  */
  struct LirRhsHash {
    std::size_t operator()(const LirNodePtr& node) const {
      if(!node) return 0;
      std::size_t h1 = std::hash<LirKind>()(node->opcode);       
      std::size_t h2 = std::hash<LirNodePtr>()(node->a);
      std::size_t h3 = std::hash<LirNodePtr>()(node->b);
      return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
  };

  struct LirRhsEqual {
    bool operator()(const LirNodePtr& lhs, const LirNodePtr& rhs) const {
      if(!lhs || !rhs) return lhs == rhs;      
      if(lhs->opcode != rhs->opcode) return false;
      
      return (lhs->a == rhs->a) && (lhs->b == rhs->b); 
    }
  };

  static bool common_subexpression_elimination(BasicBlockPtr& bb){   
    bool changed = false;
    using CseTable = std::unordered_set<LirNodePtr, LirRhsHash, LirRhsEqual>;
    CseTable table;

    for(auto iter = bb->insts.begin(); iter != bb->insts.end();){
      auto inst = *iter;
      
      //generate-----
      if(is_binary_opcode(inst->opcode)){
	if(table.contains(inst)){
	  auto it = table.find(inst);
	  auto av_inst = *it; //av_inst: t1=a+b

	  //replace
	  //inst: d=a+b --> d=t1
	  const auto mov_node = make_node(LirKind::LIR_MOV,
					  inst->d,
					  nullptr,
					  av_inst->d);
	  iter = bb->insts.erase(iter);
	  iter = bb->insts.insert(iter, mov_node);
	  changed = true;
	} else {
	  //register this instruction in table
	  table.insert(inst);
	}
      } //if is_binary_opcode()
      //-------------
      
      //kill---------
      //inst: d=a+b --> kills expressions containing "d"
      const auto def = (*iter)->d;
      for(auto table_it = table.begin(); table_it != table.end();){
	auto av_inst = *table_it;
	bool kill_flag = false;
	for(const auto& use: av_inst->Uses()){
	  if(use == def){
	    kill_flag = true;
	    break;
	  }
	}

	if(kill_flag){
	  table_it = table.erase(table_it);
	} else {
	  table_it++;
	}	
      } //for table_it      
      //-------------
      iter++;
    } //for inst
    
    return changed;
  }
  
  static bool constant_propagation(BasicBlockPtr& bb){
    bool changed = false;
    std::unordered_map<LirNodePtr, int64_t, LirSharedPtrHash> table;

    for(auto inst: bb->insts){
      //generation-------------------
      if(is_imm(inst)){
	table.insert_or_assign(inst->d, inst->imm);
	continue;
      }
      if(inst->opcode == LirKind::LIR_MOV
	 && is_imm(inst->b)){
	table.insert_or_assign(inst->d, inst->b->imm);
	continue;
      }
      //-----------------------------

      //replace----------------------
      for(auto& use: inst->Uses()){
	if(table.contains(use) && !is_imm(use)){
	  use->opcode = LirKind::LIR_IMM;
	  use->imm = table.find(use)->second;
	  changed = true;
	}
      }
      /*
      //cast
      if(inst->opcode == LirKind::LIR_CAST){
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
      */
    } //for inst
    return changed;
  }

  static bool copy_propagation(BasicBlockPtr& bb){
    bool changed = false;
    std::unordered_map<LirNodePtr, LirNodePtr, LirSharedPtrHash> table; //d --> b

    for(auto inst: bb->insts){
      //generation and kill-------------------
      if(inst->opcode == LirKind::LIR_MOV
	 && !is_imm(inst->b)
	 && !inst->b->is_fixed_reg){
	table.insert_or_assign(inst->d, inst->b);
	continue;
      }
      //-----------------------------           
      /*
      for(auto& use: inst->get_mutable_uses()){
	if(table.contains(use) && !is_imm(use)){
	  use = table.find(use)->second;
	  changed = true;
	}
      }
      for(auto& def: inst->get_mutable_defs()){
	table.erase(def);
      }
      */
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
      if(inst->opcode == LirKind::LIR_IMM
	 || inst->opcode == LirKind::LIR_LABEL_ADDR
	 || inst->opcode == LirKind::LIR_LOAD_STACK
	 || inst->opcode == LirKind::LIR_LOAD_LABEL
	 || inst->opcode == LirKind::LIR_LVAR
	 || inst->opcode == LirKind::LIR_LOAD_SPILL
	 ){
	table.erase(inst->d);
      }
      
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
	 || inst->opcode == LirKind::LIR_STORE_STACK
	 || inst->opcode == LirKind::LIR_STORE_LABEL){
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
	table.erase(inst->d);
      }
      
    } //for inst
    
    return changed;
  }

  int64_t calc_constant(const LirNodePtr& inst){
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

  static void constant_folding(std::list<LirNodePtr>::iterator& iter,
			       BasicBlockPtr& bb){
    auto& inst = *iter;
    const int64_t c = calc_constant(inst);
    auto imm_node = make_imm_node(c);
    imm_node->d = inst->d;
    
    iter = bb->insts.erase(iter);
    iter = bb->insts.insert(iter, imm_node);	  
  }

  static bool constant_foldings(BasicBlockPtr& bb){
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
  
  static bool peephole(BasicBlockPtr& bb){
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

      if(inst->opcode == LirKind::LIR_PTR_ADD
	 || inst->opcode == LirKind::LIR_PTR_SUB){
	if(is_imm(inst->b)
	   && inst->b->imm == 0){
	  //d=a+0 --> d=a
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
      } //if LIR_PTR_ADD

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
  
  static bool propagate_address(BasicBlockPtr& bb){
    //stack address (local variable)
    //v1 <- [rbp-4]
    //v2 <- [v1]
    //-->
    //v2 <- [rbp-4] (movsxd v2, dword ptr [rbp-4])

    //label address (global variable)
    //v1 <- g
    //v2 <- [v1]
    //-->
    //v2 <- [g] (movsxd v2, dword ptr [g])
    
    bool changed = false;
    std::unordered_map<LirNodePtr, std::shared_ptr<Lunaria::Var>, LirSharedPtrHash> st_offset_table; //vn --> lvar
    std::unordered_map<LirNodePtr, std::string, LirSharedPtrHash> label_table; //vn --> label
    for(auto iter = bb->insts.begin(); iter != bb->insts.end(); ++iter){
      auto inst = *iter;
      if(inst->opcode == LirKind::LIR_LVAR){
	st_offset_table.insert_or_assign(inst->d, inst->lvar);
      }

      if(inst->opcode == LirKind::LIR_LABEL_ADDR){
	label_table.insert_or_assign(inst->d, inst->name);
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
	else if(label_table.contains(inst->b)){
	  auto node = make_node(LirKind::LIR_LOAD_LABEL,
				inst->d,
				nullptr,
				nullptr);
	  node->name = label_table.find(inst->b)->second;
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
	else if(label_table.contains(inst->a)){
	  auto node = make_node(LirKind::LIR_STORE_LABEL,
				nullptr,
				nullptr,
				inst->b);
	  node->name = label_table.find(inst->a)->second;
	  node->type_size = inst->type_size;
	  iter = bb->insts.erase(iter);
	  iter = bb->insts.insert(iter, node);
	  changed = true;
	}
      }
    }
    return changed;
  }

  static bool redundant_load_elimination_stack(BasicBlockPtr& bb){
    //redundant load elimination
    //Load_stack: v1 <- [rbp-4]
    //Load_stack: v2 <- [rbp-4] ==> v2 <- v1

    //store-to-load forwarding
    //[rbp-8] <- v3
    //v4 <- [rbp-8] ==> v4 <- v3 (or Cast: v5(64bit) <- v3(32bit); v4 <- v5; if store to rbp-8 is 32bit)

    bool changed = false;
    using Table = std::unordered_map<int, LirNodePtr>;
    Table st_offset_table; //stack_offset --> vn (Load)
    Table store_table;     //stack_offset --> inst (Store)
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
	  LirNodePtr node = make_node(LirKind::LIR_CAST,
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

  static bool redundant_load_elimination_label(BasicBlockPtr& bb){
    //redundant load elimination
    //Load_label: v1 <- [g]
    //Load_label: v2 <- [g] ==> v2 <- v1

    //store-to-load forwarding
    //[g] <- v3
    //v4 <- [g] ==> v4 <- v3 (or Cast: v5(64bit) <- v3(32bit); v4 <- v5; if store to g is 32bit)

    bool changed = false;
    using Table = std::unordered_map<std::string, LirNodePtr>;
    Table load_table;  //label --> vn (Load)
    Table store_table; //label --> inst (Store)    
    for(auto iter = bb->insts.begin(); iter != bb->insts.end(); ++iter){
      auto inst = *iter;
      if(inst->opcode == LirKind::LIR_LOAD_LABEL){
	if(load_table.contains(inst->name)){
	  auto mov_node = make_node(LirKind::LIR_MOV,
				    inst->d,
				    nullptr,
				    load_table.find(inst->name)->second);
	  iter = bb->insts.erase(iter);
	  iter = bb->insts.insert(iter, mov_node);
	  changed = true;
	}
	else if(store_table.contains(inst->name)){
	  
	  auto store_inst = store_table.find(inst->name)->second;
	  LirNodePtr node = make_node(LirKind::LIR_CAST,
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
	  load_table.insert(std::make_pair(inst->name, inst->d));
	}
      } //if(inst->opcode == LirKind::LIR_LOAD_LABEL)

      if(inst->opcode == LirKind::LIR_STORE_LABEL){	
	  store_table.insert_or_assign(inst->name, inst);
	  load_table.erase(inst->name);
      }

      if(inst->opcode == LirKind::LIR_FUNCALL
	 || inst->opcode == LirKind::LIR_STORE){
	load_table.clear();
	store_table.clear();
      }      
    } //for iter
    return changed;
  }
  
  bool optimize_bb(BasicBlockPtr& bb){
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
    changed = changed || propagate_address(bb);
    changed = changed || redundant_load_elimination_stack(bb);
    changed = changed || redundant_load_elimination_label(bb);
    changed = changed || common_subexpression_elimination(bb);
    return changed;
  }
} //namespace Lunaria::LIR::Optimizer
