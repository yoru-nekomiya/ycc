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
collectReg(std::unique_ptr<myLIR::Program>& prog){
  std::list<std::shared_ptr<myLIR::LirNode>> listReg;
  int instCount = 1;
  for(auto& fn: prog->fns){
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
  } //for fn
  return listReg;
}

static std::unordered_map<int, std::shared_ptr<myLIR::LirNode>> used;

static void allocate(std::unique_ptr<myLIR::Program>& prog){
  const auto listReg = collectReg(prog);

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
    std::cerr << "reg alloc failed! spill code" << std::endl;
    exit(1);
  } //for
}

void allocateRegister_x86_64(std::unique_ptr<myLIR::Program>& prog){
  convertThreeAddress2Two(prog);
  allocate(prog);
}

} //namespace myRegAlloc
