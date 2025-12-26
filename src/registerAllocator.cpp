#include "ycc.hpp"

namespace myRegAlloc {
  const int num_reg = 7;

static void convertThreeAddress2Two(std::unique_ptr<myLIR::Program>& prog){
  //d = a op b; --> d = a; d = d op b;
  for(auto& fn: prog->fns){
    for(auto& bb: fn->bbs){
      for(auto iter = bb->insts.begin(); iter != bb->insts.end(); iter++){
	if(!(*iter)->d || !(*iter)->a){
	  continue;
	}
	auto movNode = std::make_shared<myLIR::LirNode>();
	movNode->opcode = myLIR::LirKind::LIR_MOV;
	movNode->d = (*iter)->d;
	movNode->b = (*iter)->a;
	
	(*iter)->a = (*iter)->d;
	iter = bb->insts.insert(iter, std::move(movNode));
	//Now, iter points movNode, so increment iter
	iter++;
      } //for iter
    } //for bbs
  } //for fn
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
  //for(auto& fn: prog->fns){
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
    //} //for fn
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
  
static void allocate(std::list<std::shared_ptr<myLIR::LirNode>>& listReg){
  //const auto listReg = collectReg(prog);

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
      break;
    }
    if(found) continue;

    //spill code
    /*
    std::cerr << "reg alloc failed! spill code" << std::endl;
    exit(1);
    */
    used[num_reg-1] = reg;
    const int k = choose_to_spill();
    reg->rn = k;
    used[k]->rn = num_reg-1;
    used[k]->spill = true;
    used[k] = reg;
  } //for
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

  static int spill_num = 0;
void allocateRegister_x86_64(std::unique_ptr<myLIR::Program>& prog){
  convertThreeAddress2Two(prog);
  for(auto& fn: prog->fns){
    auto listReg = collectReg(fn);
    allocate(listReg);
    for(auto& reg: listReg){
      if(!reg->spill){
	continue;
      }
      
      const std::string name = "__tmp_lvar_spill__" + std::to_string(spill_num++);
      const auto type = Lunaria::pointer_to(Lunaria::int_type);
      auto lvar = std::make_shared<Lunaria::Var>(name, type, true);
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
