#include "ycc.hpp"

const int num_reg = 7;

static void convertThreeAddress2Two(std::list<std::shared_ptr<LirNode>>& lirList){
  //d = a op b; --> d = a; d = d op b;

  for(auto iter = lirList.begin(); iter != lirList.end(); iter++){
    if(!(*iter)->d || !(*iter)->a){
      continue;
    }
    /*
    auto movNode = std::make_shared<LirNode>(LirKind::LIR_MOV,
					     (*iter)->d,
					     nullptr,
					     (*iter)->a,
					     -1,
					     -1,
					     -1);
    */
    auto movNode = std::make_shared<LirNode>();
    movNode->opcode = LirKind::LIR_MOV;
    movNode->d = (*iter)->d;
    movNode->b = (*iter)->a;
      
    (*iter)->a = (*iter)->d;
    iter = lirList.insert(iter, std::move(movNode));
    //Now, iter points movNode, so increment iter
    iter++;
  } //for
}

static void allocate(std::list<std::shared_ptr<LirNode>>& lirList){
  int n = 0;
  for(auto& lirNode: lirList){
    if(n < num_reg){
      lirNode->d->rn = n++;
      lirNode->rn = lirNode->d->rn;
    } else {
      std::cerr << "reg alloc failed!" << std::endl;
      exit(1);
    }
  }
}

void allocateRegister_x86_64(std::list<std::shared_ptr<LirNode>>& lirList){
  convertThreeAddress2Two(lirList);
  allocate(lirList);
}
