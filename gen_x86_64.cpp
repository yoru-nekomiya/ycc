#include "ycc.hpp"

std::string regs[] = {"r10", "r11", "rbx", "r12", "r13", "r14", "r15"};
const std::string regs8[] = {"r10b", "r11b", "bl", "r12b", "r13b", "r14b", "r15b"};

static void print_cmp(const std::string& cmp,
		      const std::shared_ptr<LirNode>& lirNode){
  //print instructions for LT, LE, EQ, NE
  const int d = lirNode->d ? lirNode->d->rn : 0;
  const int a = lirNode->a ? lirNode->a->rn : 0;
  const int b = lirNode->b ? lirNode->b->rn : 0;

  std::cout << "  cmp " << regs[a] << ", " << regs[b] << std::endl; //the result is stored in the flag register
  std::cout << "  " << cmp << " " << regs8[d] << std::endl; //store the result stored in the flag register into an 8-bit register
  std::cout << "  movzb " << regs[d] << ", " << regs8[d] << std::endl; //clear the upper 56 bits of the 64-bit register to zero
}

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
  case LirKind::LIR_LT:
    print_cmp("setl", lirNode);
    break;
  case LirKind::LIR_LE:
    print_cmp("setle", lirNode);
    break;
  case LirKind::LIR_EQ:
    print_cmp("sete", lirNode);
    break;
  case LirKind::LIR_NE:
    print_cmp("setne", lirNode);
    break;
  case LirKind::LIR_LVAR:
    std::cout << "  lea " << regs[d] << ", [rbp-" << lirNode->lvar->offset << "]" << std::endl;
    break;
  case LirKind::LIR_LOAD:
    std::cout << "  mov " << regs[d] << ", [" << regs[b] << "]" << std::endl;
    break;
  case LirKind::LIR_STORE:
    std::cout << "  mov [" << regs[a] << "], " << regs[b] << std::endl;
    break;
  case LirKind::LIR_RETURN:
    std::cout << "  mov rax, " << regs[a] << std::endl
	      << "  mov rsp, rbp" << std::endl
	      << "  pop rbp" << std::endl
	      << "  ret" << std::endl;
    break;
  case LirKind::LIR_BR:
    std::cout << "  cmp " << regs[b] << ", 0" << std::endl;
    std::cout << "  jne .L" << lirNode->bb1->label << std::endl;
    std::cout << "  jmp .L" << lirNode->bb2->label << std::endl;
    break;
  case LirKind::LIR_JMP:
    std::cout << "  jmp .L" << lirNode->bb1->label << std::endl;
    break;
  case LirKind::LIR_FUNCALL:
    std::cout << "  push r10" << std::endl
	      << "  push r11" << std::endl
	      << "  mov rax, 0" << std::endl
	      << "  call " << lirNode->funcName << std::endl
	      << "  pop r11" << std::endl
	      << "  pop r10" << std::endl
	      << "  mov " << regs[d] << ", rax" << std::endl;
    break;
  } //switch
}

void gen_x86_64(const std::list<std::shared_ptr<BasicBlock>>& bbList){
  std::cout << ".intel_syntax noprefix" << std::endl
	    << ".global main" << std::endl
	    << "main:" << std::endl;

  const int offset = localVars.size() % 2 == 0 ?
    localVars.size()*8 + 8 : localVars.size()*8; //16 byte alignment
  std::cout << "  push rbp" << std::endl
	    << "  mov rbp, rsp" << std::endl
	    << "  sub rsp, " << offset << std::endl;
  
  for(const auto& bb: bbList){
    std::cout << ".L" << bb->label << ":" << std::endl;
    for(const auto& lirNode: bb->insts){
      gen(lirNode);
    }
  }
}
