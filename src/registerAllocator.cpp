#include "ycc.hpp"
#include "util.hpp"
#include "optimization/opt_utils.hpp"

namespace myRegAlloc {
  const int num_reg = 7; //"r10", "r11", "rbx", "r12", "r13", "r14", "r15"

  static void insert_64bit_imm_mov(std::list<std::shared_ptr<myLIR::LirNode>>::iterator& iter,
				   std::shared_ptr<myLIR::BasicBlock>& bb){
    //Check if operand "b" is 64-bit immediate value.
    //If so, insert the MOV instruction to load the value in 64-bit register
    if(!(*iter)->b) return;
    if(myLIR::is_imm((*iter)->b) && !myLIR::is_int32((*iter)->b)){
      auto d = myLIR::new_reg("", 8);
      auto imm_node = std::make_shared<myLIR::LirNode>();
      imm_node->opcode = myLIR::LirKind::LIR_IMM;
      imm_node->d = std::move(d);
      imm_node->imm = (*iter)->b->imm;
      
      (*iter)->b = imm_node->d;
      iter = bb->insts.insert(iter, imm_node);
      iter++;
    }
  }

  static void
  insert_32bit_imm_mov_for_idiv(std::list<std::shared_ptr<myLIR::LirNode>>::iterator& iter,
				std::shared_ptr<myLIR::BasicBlock>& bb){
    //Check if an instruction is LIR_DIV or LIR_REM,
    //and check if operand "b" is 32-bit immediate value.
    //If so, insert the MOV instruction to load the value in register
    if((*iter)->opcode == myLIR::LirKind::LIR_DIV
       || (*iter)->opcode == myLIR::LirKind::LIR_REM){     
      if(myLIR::is_imm((*iter)->b) && myLIR::is_int32((*iter)->b)){
	auto d = myLIR::new_reg("", 4);
	auto imm_node = std::make_shared<myLIR::LirNode>();
	imm_node->opcode = myLIR::LirKind::LIR_IMM;
	imm_node->d = std::move(d);
	imm_node->imm = (*iter)->b->imm;
      
	(*iter)->b = imm_node->d;
	iter = bb->insts.insert(iter, imm_node);
	iter++;
      } //if b
    } //if DIV or REM
  }

  static void
  decompose_ptr_add_and_sub(std::list<std::shared_ptr<myLIR::LirNode>>::iterator& iter,
			    std::shared_ptr<myLIR::BasicBlock>& bb){
    //Decompose PTR_ADD and PTR_SUB because they need two destination registers.
    auto inst = *iter;
    if(inst->opcode == myLIR::LirKind::LIR_PTR_ADD
       || inst->opcode == myLIR::LirKind::LIR_PTR_SUB){
      const int s = inst->type_base_size;
      if(s != 1 && s != 2 && s != 4 && s != 8){
	if(!(is_imm(inst->b) && is_int32(inst->b))){
	  auto d = myLIR::new_reg("", 8);
	  auto mul_node = myLIR::opt::make_node(myLIR::LirKind::LIR_MUL,
						d,
						inst->b,
						myLIR::opt::make_imm_node(s));
	  std::shared_ptr<myLIR::LirNode> node = nullptr;
	  if(inst->opcode == myLIR::LirKind::LIR_PTR_ADD){
	    node = myLIR::opt::make_node(myLIR::LirKind::LIR_ADD,
					 inst->d,
					 inst->a,
					 mul_node->d);
	  }
	  else if(inst->opcode == myLIR::LirKind::LIR_PTR_SUB){
	    node = myLIR::opt::make_node(myLIR::LirKind::LIR_SUB,
					 inst->d,
					 inst->a,
					 mul_node->d);
	  }
	  iter = bb->insts.erase(iter);
	  iter = bb->insts.insert(iter, mul_node);
	  ++iter;
	  iter = bb->insts.insert(iter, node);
	  --iter;
	}
      }
    }
    
  }
  
  static void convert_3ac_to_2ac(std::list<std::shared_ptr<myLIR::LirNode>>::iterator& iter,
				 std::shared_ptr<myLIR::BasicBlock>& bb){
  //convert 3-address code to 2-address code
  //d = a op b; --> d = a; d = d op b;
  if(!(*iter)->d || !(*iter)->a) return;	
  auto movNode = std::make_shared<myLIR::LirNode>();
  movNode->opcode = myLIR::LirKind::LIR_MOV;
  movNode->d = (*iter)->d;
  //movNode->d = myLIR::new_reg("", 8);
  movNode->b = (*iter)->a;
  
  (*iter)->a = (*iter)->d;
  //(*iter)->a = movNode->d;
  iter = bb->insts.insert(iter, std::move(movNode));
  //Now, iter points movNode, so increment iter
  iter++;
}

static void setLastUse(std::shared_ptr<myLIR::LirNode>& lirNode,
		       int c){
  if(lirNode && lirNode->lastUse < c){
    lirNode->lastUse = c;
  }
}

static std::list<std::shared_ptr<myLIR::LirNode>>
collectReg(std::shared_ptr<myLIR::Function>& fn){
  std::list<std::shared_ptr<myLIR::LirNode>> listReg;
  int instCount = 1;
  for(auto& bb: fn->bbs){
    if(bb->param){
      bb->param->def = instCount;
      listReg.push_back(bb->param);
      instCount++;
    }
    for(auto& lirNode: bb->insts){
      if(lirNode->d && !lirNode->d->def){
	lirNode->d->def = instCount;
	listReg.push_back(lirNode->d);
      }
      setLastUse(lirNode->a, instCount);
      setLastUse(lirNode->b, instCount);
      setLastUse(lirNode->bbarg, instCount);
      
      if(lirNode->opcode == myLIR::LirKind::LIR_FUNCALL){
	for(auto& n: lirNode->args){
	  setLastUse(n, instCount);
	}
      }
      
      instCount++;
    }
  } //for bb
  return listReg;
} 
  
static std::unordered_map<int, std::shared_ptr<myLIR::LirNode>> used;

  static int choose_to_spill() {
    int k = 0;
    for (int i = 1; i < num_reg; i++)
      if (used[k]->lastUse < used[i]->lastUse)
	k = i;
    return k;
  }
  
static int allocate(std::list<std::shared_ptr<myLIR::LirNode>>& listReg){
  int max_reg_pressure = INT_MIN;
  for(int i = 0; i < num_reg; i++){
    used[i] = nullptr;
  }
  
  for(auto& reg: listReg){
    bool found = false;
    for(int i = 0; i < num_reg-1; i++){
      if(used[i] && reg->def < used[i]->lastUse){
	continue;
      }
      reg->rn = i;
      used[i] = reg;
      found = true;
      max_reg_pressure = std::max(max_reg_pressure, i);
      break;
    }
    if(found) continue;

    //spill code    
    used[num_reg-1] = reg;
    const int k = choose_to_spill();
    reg->rn = k;
    used[k]->rn = num_reg-1;
    used[k]->spill = true;
    used[k] = reg;
    max_reg_pressure = num_reg-1;
  } //for
  return max_reg_pressure;
}
  
  static void spill_store(std::list<std::shared_ptr<myLIR::LirNode>>& insts,
			  const std::shared_ptr<myLIR::LirNode>& ir){
    const auto r = ir->d;
    if(!r || !r->spill){
      return;
    }

    auto ir2 = std::make_shared<myLIR::LirNode>();
    ir2->opcode = myLIR::LirKind::LIR_STORE_SPILL;
    ir2->a = r;
    ir2->lvar = r->lvar;
    insts.push_back(ir2);
  }

  static void spill_load(std::list<std::shared_ptr<myLIR::LirNode>>& insts,
			 const std::shared_ptr<myLIR::LirNode>& r){
    if(!r || !r->spill){
      return;
    }

    auto ir2 = std::make_shared<myLIR::LirNode>();
    ir2->opcode = myLIR::LirKind::LIR_LOAD_SPILL;
    ir2->d = r;
    ir2->lvar = r->lvar;
    insts.push_back(ir2);
  }

  static void emit_spill_code(std::shared_ptr<myLIR::BasicBlock>& bb){
    std::list<std::shared_ptr<myLIR::LirNode>> insts;
    for(auto& inst: bb->insts){
      spill_load(insts, inst->a);
      spill_load(insts, inst->b);
      spill_load(insts, inst->bbarg);
      insts.push_back(inst);
      spill_store(insts, inst);
    } //for inst
    bb->insts.clear();
    bb->insts.resize(insts.size());
    std::copy(insts.begin(), insts.end(), bb->insts.begin());
  }

  void preprocess_x86_64(std::unique_ptr<myLIR::Program>& prog){
    for(auto& fn: prog->fns){
      for(auto& bb: fn->bbs){
	for(std::list<std::shared_ptr<myLIR::LirNode>>::iterator iter = bb->insts.begin(); iter != bb->insts.end(); iter++){
	  insert_64bit_imm_mov(iter, bb);
	  insert_32bit_imm_mov_for_idiv(iter, bb);
	  decompose_ptr_add_and_sub(iter, bb);
	  convert_3ac_to_2ac(iter, bb);
	}
      }
    }
  }
  
  static int spill_num = 0;
void allocateRegister_x86_64(std::unique_ptr<myLIR::Program>& prog){
  preprocess_x86_64(prog);
  for(auto& fn: prog->fns){
    auto listReg = collectReg(fn);
    fn->max_reg_pressure = allocate(listReg);
    prog->funcname_to_reg_pressure.insert(std::make_pair(fn->name, fn->max_reg_pressure));
    for(auto& reg: listReg){
      if(!reg->spill){
	continue;
      }
      
      const std::string name = "__tmp_lvar_spill__" + std::to_string(spill_num++);
      const auto type = Lunaria::pointer_to(Lunaria::int_type);
      auto lvar = std::make_shared<Lunaria::Var>(name, type, true);
      lvar->id = fn->localVars.size() + 1;
      reg->lvar = lvar;
      fn->localVars.insert(lvar);
      std::cerr << "--- spilled register in function " << fn->name << std::endl;
    } //for reg

    for(auto& bb: fn->bbs){
      emit_spill_code(bb);
    } //for bb    
  } //for fn
}

} //namespace myRegAlloc
