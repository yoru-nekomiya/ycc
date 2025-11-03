#include "ycc.hpp"

namespace myCodeGen {
  static const std::string regs8[] = {"r10b", "r11b", "bl", "r12b", "r13b", "r14b", "r15b"};
  static const std::string regs16[] = {"r10w", "r11w", "bx", "r12w", "r13w", "r14w", "r15w"};
  static const std::string regs32[] = {"r10d", "r11d", "ebx", "r12d", "r13d", "r14d", "r15d"};
  static const std::string regs[] = {"r10", "r11", "rbx", "r12", "r13", "r14", "r15"};

  static const std::string argregs8[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
  static const std::string argregs16[] = {"di", "si", "dx", "cx", "r8w", "r9w"};
  static const std::string argregs32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"}; 
  static const std::string argregs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
  
static std::string funcname = "";

  static std::string reg(int r, int size){
    if(size == 1){
      return regs8[r];
    }
    if(size == 2){
      return regs16[r];
    }
    if(size == 4){
      return regs32[r];
    }
    assert(size == 8);
    return regs[r];
  }

  static std::string argreg(int r, int size){
    if(size == 1){
      return argregs8[r];
    }
    if(size == 2){
      return argregs16[r];
    }
    if(size == 4){
      return argregs32[r];
    }
    assert(size == 8);
    return argregs[r];
  }
  
static void print_cmp(const std::string& cmp,
		      const std::shared_ptr<myLIR::LirNode>& lirNode){
  //print instructions for LT, LE, EQ, NE
  const int d = lirNode->d ? lirNode->d->rn : 0;
  const int a = lirNode->a ? lirNode->a->rn : 0;
  const int b = lirNode->b ? lirNode->b->rn : 0;

  std::cout << "  cmp " << regs[a] << ", " << regs[b] << std::endl; //the result is stored in the flag register
  std::cout << "  " << cmp << " " << regs8[d] << std::endl; //store the result stored in the flag register into an 8-bit register
  std::cout << "  movzb " << regs[d] << ", " << regs8[d] << std::endl; //clear the upper 56 bits of the 64-bit register to zero
}

  static void load(const std::shared_ptr<myLIR::LirNode>& lirNode){
    const int d = lirNode->d ? lirNode->d->rn : 0;
    const int b = lirNode->b ? lirNode->b->rn : 0;
    const int size = lirNode->type_size;
    //signed extension
    if(size == 1){
      std::cout << "  movsx " << regs[d] << ", byte ptr [" << regs[b] << "]" << std::endl;
    } else if(size == 2){
      std::cout << "  movsx " << regs[d] << ", word ptr [" << regs[b] << "]" << std::endl;
    } else if(size == 4){
      std::cout << "  movsxd " << regs[d] << ", dword ptr [" << regs[b] << "]" << std::endl;
    } else {
      assert(size == 8);
      std::cout << "  mov " << regs[d] << ", [" << regs[b] << "]" << std::endl;
    }
  }

static void gen(const std::shared_ptr<myLIR::LirNode>& lirNode){
  const int d = lirNode->d ? lirNode->d->rn : 0;
  const int a = lirNode->a ? lirNode->a->rn : 0;
  const int b = lirNode->b ? lirNode->b->rn : 0;

  switch(lirNode->opcode){
  case myLIR::LirKind::LIR_MOV:
    std::cout << "  mov " << regs[d] << ", " << regs[b] << std::endl;
    break;
  case myLIR::LirKind::LIR_IMM:
    std::cout << "  mov " << regs[d] << ", " << lirNode->imm << std::endl;
    break;
  case myLIR::LirKind::LIR_ADD:
    std::cout << "  add " << regs[d] << ", " << regs[b] << std::endl;
    break;
  case myLIR::LirKind::LIR_SUB:
    std::cout << "  sub " << regs[d] << ", " << regs[b] << std::endl;
    break;
  case myLIR::LirKind::LIR_MUL:
    /*
    std::cout << "  mov rax, " << regs[b] << std::endl;
    std::cout << "  imul " << regs[d] << std::endl;
    std::cout << "  mov " << regs[d] << ", rax" << std::endl;*/
    //for 64 bit
    //In this style, discard the upper 64 bits of the result
    std::cout << "  imul " << regs[d] << ", " << regs[b] << std::endl;
    //TODO: for 128 bit
    break;
  case myLIR::LirKind::LIR_DIV:
    std::cout << "  mov rax, " << regs[d] << std::endl;
    std::cout << "  cqo" << std::endl;
    std::cout << "  idiv " << regs[b] << std::endl;
    std::cout << "  mov " << regs[d] << ", rax" << std::endl;
    break;
  case myLIR::LirKind::LIR_LT:
    print_cmp("setl", lirNode);
    break;
  case myLIR::LirKind::LIR_LE:
    print_cmp("setle", lirNode);
    break;
  case myLIR::LirKind::LIR_EQ:
    print_cmp("sete", lirNode);
    break;
  case myLIR::LirKind::LIR_NE:
    print_cmp("setne", lirNode);
    break;
  case myLIR::LirKind::LIR_LVAR:
    std::cout << "  lea " << regs[d] << ", [rbp-" << lirNode->lvar->offset << "]" << std::endl;
    break;
  case myLIR::LirKind::LIR_LABEL_ADDR:
    std::cout << "  lea " << regs[d] << ", " << lirNode->name << std::endl;
    break;
  case myLIR::LirKind::LIR_LOAD:
    //std::cout << "  mov " << reg(d, lirNode->type_size) << ", [" << regs[b] << "]" << std::endl;
    load(lirNode);
    break;
  case myLIR::LirKind::LIR_STORE:
    std::cout << "  mov [" << regs[a] << "], " << reg(b, lirNode->type_size) << std::endl;
    break;
  case myLIR::LirKind::LIR_STORE_ARG:
    std::cout << "  mov [rbp-" << lirNode->lvar->offset
	      << "], "
	      << argreg(lirNode->imm, lirNode->type_size)
	      << std::endl;
    break;
  case myLIR::LirKind::LIR_RETURN:
    //std::cout << "  mov rax, " << regs[a] << std::endl;
    if(lirNode->a){
      std::cout << "  mov rax, " << regs[a] << std::endl;
    }
    std::cout << "  jmp .L.return." << funcname << std::endl;
    
      break;
  case myLIR::LirKind::LIR_BR:
    std::cout << "  cmp " << regs[b] << ", 0" << std::endl;
    std::cout << "  jne .L" << lirNode->bb1->label << std::endl;
    std::cout << "  jmp .L" << lirNode->bb2->label << std::endl;
    break;
  case myLIR::LirKind::LIR_JMP:
    std::cout << "  jmp .L" << lirNode->bb1->label << std::endl;
    break;
  case myLIR::LirKind::LIR_FUNCALL:
    for(int i = 0; i < lirNode->args.size(); i++){
      std::cout << "  mov " << argregs[i] << ", "
		<< regs[lirNode->args[i]->rn] << std::endl;
    }
    std::cout << "  push r10" << std::endl
	      << "  push r11" << std::endl
	      << "  mov rax, 0" << std::endl
	      << "  call " << lirNode->funcName << std::endl
	      << "  pop r11" << std::endl
	      << "  pop r10" << std::endl
	      << "  mov " << regs[d] << ", rax" << std::endl;
    break;
  case myLIR::LirKind::LIR_PTR_ADD: {
    /*
    std::cout << "  imul " << regs[b] << ", " << lirNode->type_base_size << std::endl;
    std::cout << "  add " << regs[d] << ", " << regs[b] << std::endl;
    */
    const int size = lirNode->type_base_size;
    if(size == 1){
      std::cout << "  add " << regs[d] << ", " << regs[b] << std::endl;
    } else if(size == 2 || size == 4 || size == 8){
      std::cout << "  lea " << regs[d] << ", ["
		<< regs[d] << "+" << regs[b] << "*" << size
		<< "]" << std::endl;
    }
    break;
  }
  case myLIR::LirKind::LIR_PTR_SUB: {
    /*
    std::cout << "  imul " << regs[b] << ", " << lirNode->type_base_size << std::endl;
    std::cout << "  sub " << regs[d] << ", " << regs[b] << std::endl;
    */
    const int size = lirNode->type_base_size;
    if(size == 1){
      std::cout << "  sub " << regs[d] << ", " << regs[b] << std::endl;
    } else if(size == 2 || size == 4 || size == 8){
      std::cout << "  neg " << regs[b] << std::endl;
      std::cout << "  lea " << regs[d] << ", ["
		<< regs[d] << "+" << regs[b] << "*" << size
		<< "]" << std::endl;
    }
    break;
  }
  case myLIR::LirKind::LIR_PTR_DIFF:
    std::cout << "  sub " << regs[d] << ", " << regs[b] << std::endl;
    std::cout << "  mov rax, " << regs[d] << std::endl;
    std::cout << "  cqo" << std::endl;
    std::cout << "  mov " << regs[b] << ", " << lirNode->type_base_size << std::endl;
    std::cout << "  idiv " << regs[b] << std::endl;
    std::cout << "  mov " << regs[d] << ", rax" << std::endl;
    break;
  } //switch
}

  static void print_literal(const std::string& literal){
    std::string out;
    out.reserve(literal.size());
    for(char c: literal){
      switch(c){
      case '\n': out += "\\n"; break;
      case '\t': out += "\\t"; break;
      case '\r': out += "\\r"; break;
      case '\\': out += "\\\\"; break;
      case '\'': out += "\\\'"; break;
      case '\"': out += "\\\""; break;
      case '\v': out += "\\v"; break;
      case '\b': out += "\\b"; break;
      case '\f': out += "\\f"; break;
      case '\a': out += "\\a"; break;	
      case '\?': out += "\\\?"; break;
      default:
	if(c < 32 || c >= 127){
	  char buf[8];
	  std::snprintf(buf, sizeof(buf), "\\%03o", c);
	  out += buf;
	} else {
	  out += c;
	}
	break;
      }
    }
    std::cout << "  .string \"" << out << "\"\n";
  }

  static void emit_data(const std::unique_ptr<myLIR::Program>& prog){
    std::cout << ".bss" << std::endl;

    for(const auto& [name, gvar]: prog->globalVars){
      if(gvar->isLiteral){
	continue;
      }
      std::cout << ".align " << gvar->type->align << std::endl;
      std::cout << gvar->name << ":\n";
      std::cout << "  .zero " << gvar->type->size << std::endl;
    }

    std::cout << ".data\n";
    for(const auto& [name, gvar]: prog->globalVars){
      if(gvar->isLiteral){
	std::cout << ".align " << gvar->type->align << std::endl;
	std::cout << gvar->name << ":\n";
	print_literal(gvar->literal);
      }
    }
  }
  
static void emit_text(const std::unique_ptr<myLIR::Program>& prog){
  std::cout << ".text" << std::endl;

  for(const auto& fn: prog->fns){
    std::cout << ".global " << fn->name << std::endl
	      << fn->name << ":" << std::endl;
    funcname = fn->name;

    //calculate stack size
    int offset = 0;
    for(const auto& [name, lvar]: fn->localVars){
      offset = Lunaria::align_to(offset, lvar->type->align);
      offset += lvar->type->size;
      lvar->offset = offset;
    }
    fn->stackSize = offset % 16 == 0 ? offset + 8 : offset; //16 byte alignment

    //prologue
    std::cout << "  push rbp" << std::endl
	      << "  mov rbp, rsp" << std::endl
	      << "  sub rsp, " << fn->stackSize << std::endl;
    std::cout << "  push r12" << std::endl
	      << "  push r13" << std::endl
	      << "  push r14" << std::endl
	      << "  push r15" << std::endl
	      << "  push rbx" << std::endl;

    //generate code
    for(const auto& bb: fn->bbs){
      std::cout << ".L" << bb->label << ":" << std::endl;
      for(const auto& lirNode: bb->insts){
	gen(lirNode);
      }
    }
    
    //epilogue
    std::cout << ".L.return." << funcname << ":" << std::endl;
    std::cout << "  pop rbx" << std::endl
	      << "  pop r15" << std::endl
	      << "  pop r14" << std::endl
	      << "  pop r13" << std::endl
	      << "  pop r12" << std::endl;
    std::cout << "  mov rsp, rbp" << std::endl
	      << "  pop rbp" << std::endl
	      << "  ret" << std::endl;
  } //for fn
}

void gen_x86_64(const std::unique_ptr<myLIR::Program>& prog){
  std::cout << ".intel_syntax noprefix" << std::endl;
  emit_data(prog);
  emit_text(prog);
}

} //namespace myCodeGen
