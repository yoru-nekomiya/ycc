#include "ycc.hpp"

std::string regs[] = {"r10", "r11", "rbx", "r12", "r13", "r14", "r15"};

void gen(const std::shared_ptr<LirNode>& lirNode){
  const int d = lirNode->d ? lirNode->d->rn : 0;
  const int a = lirNode->a ? lirNode->a->rn : 0;
  const int b = lirNode->b ? lirNode->b->rn : 0;

  switch(lirNode->opcode){
  case LirKind::LIR_MOV:
    std::cout << "  mov " << regs[d] << ", " << regs[b] << std::endl;
    break;
  case LirKind::LIR_IMM:
    std::cout << "  mov " << regs[d] << ", " << lirNode->imm << std::endl;
    break;
  case LirKind::LIR_ADD:
    std::cout << "  add " << regs[d] << ", " << regs[b] << std::endl;
    break;
  case LirKind::LIR_SUB:
    std::cout << "  sub " << regs[d] << ", " << regs[b] << std::endl;
    break;
  case LirKind::LIR_MUL:
    
    std::cout << "  mov rax, " << regs[b] << std::endl;
    std::cout << "  imul " << regs[d] << std::endl;
    std::cout << "  mov " << regs[d] << ", rax" << std::endl;
    //std::cout << "  imul " << regs[d] << ", " << regs[b] << std::endl;
    break;
  case LirKind::LIR_DIV:
    std::cout << "  mov rax, " << regs[d] << std::endl;
    std::cout << "  cqo" << std::endl;
    std::cout << "  idiv " << regs[b] << std::endl;
    std::cout << "  mov " << regs[d] << ", rax" << std::endl;
    break;
  } //switch
}

void gen_x86_64(const std::list<std::shared_ptr<LirNode>>& lirList){
  for(const auto& lirNode: lirList){
    gen(lirNode);
  }
}
