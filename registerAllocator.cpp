#include "ycc.hpp"

const int num_reg = 7;

static void convertThreeAddress2Two(std::list<std::shared_ptr<BasicBlock>>& bbList){
  //d = a op b; --> d = a; d = d op b;
  for(auto& bb: bbList){
    for(auto iter = bb->insts.begin(); iter != bb->insts.end(); iter++){
      if(!(*iter)->d || !(*iter)->a){
	continue;
      }
      auto movNode = std::make_shared<LirNode>();
      movNode->opcode = LirKind::LIR_MOV;
      movNode->d = (*iter)->d;
      movNode->b = (*iter)->a;
      
      (*iter)->a = (*iter)->d;
      iter = bb->insts.insert(iter, std::move(movNode));
      //Now, iter points movNode, so increment iter
      iter++;
    } //for iter
  } //for bb
}

static void setLastUse(std::shared_ptr<LirNode>& lirNode,
		       int c){
  if(lirNode && lirNode->lastUse < c){
    lirNode->lastUse = c;
  }
}

static std::list<std::shared_ptr<LirNode>>
collectReg(std::list<std::shared_ptr<BasicBlock>>& bbList){
  std::list<std::shared_ptr<LirNode>> listReg;
  int instCount = 1;
  for(auto& bb: bbList){
    for(auto& lirNode: bb->insts){
      if(lirNode->d && !lirNode->d->def){
	lirNode->d->def = instCount;
	listReg.push_back(lirNode->d);
      }
      setLastUse(lirNode->a, instCount);
      setLastUse(lirNode->b, instCount);

      if(lirNode->opcode == LirKind::LIR_FUNCALL){
	for(auto& n: lirNode->args){
	  setLastUse(n, instCount);
	}
      }
      
      instCount++;
    }
  }
  return listReg;
}

static std::unordered_map<int, std::shared_ptr<LirNode>> used;

static void allocate(std::list<std::shared_ptr<BasicBlock>>& bbList){
  const auto listReg = collectReg(bbList);

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

void allocateRegister_x86_64(std::list<std::shared_ptr<BasicBlock>>& bbList){
  convertThreeAddress2Two(bbList);
  allocate(bbList);
}
