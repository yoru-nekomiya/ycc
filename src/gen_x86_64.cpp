#include "ycc.hpp"
#include "util.hpp"

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
    if(size == 1) return regs8[r];    
    if(size == 2) return regs16[r];    
    if(size == 4) return regs32[r];
    assert(size == 8);
    return regs[r];
  }

  static std::string argreg(int r, int size){
    if(size == 1) return argregs8[r];    
    if(size == 2) return argregs16[r];
    if(size == 4) return argregs32[r];    
    assert(size == 8);
    return argregs[r];
  }
  
static void print_cmp(const std::string& cmp,
		      const std::shared_ptr<myLIR::LirNode>& lirNode){
  //print instructions for LT, LE, EQ, NE
  const int d = lirNode->d ? lirNode->d->rn : 0;
  const int a = lirNode->a ? lirNode->a->rn : 0;
  const int b = lirNode->b ? lirNode->b->rn : 0;

  if(is_imm(lirNode->b) && is_int32(lirNode->b)){
    std::cout << std::format("  cmp {}, {}\n", regs[a], lirNode->b->imm);
  } else {
    std::cout << std::format("  cmp {}, {}\n", regs[a], regs[b]); //the result is stored in the flag register
  }
  std::cout << std::format("  {} {}\n", cmp, regs8[d]); //store the result stored in the flag register into an 8-bit register
  
  std::cout << std::format("  movzb {}, {}\n", regs[d], regs8[d]); //clear the upper 56 bits of the 64-bit register to zero
}

  static std::string get_size_qualifier(int size){
    if(size == 1) return std::string("byte ptr");    
    if(size == 2) return std::string("word ptr");    
    if(size == 4) return std::string("dword ptr");
    assert(size == 8);
    return std::string("qword ptr");
  }

  static void load(const std::shared_ptr<myLIR::LirNode>& lirNode){
    const int d = lirNode->d ? lirNode->d->rn : 0;
    const int b = lirNode->b ? lirNode->b->rn : 0;
    const int size = lirNode->type_size;
    //signed extension
    if(size == 1){
      std::cout << std::format("  movsx {}, byte ptr [{}]\n", regs[d], regs[b]);
    } else if(size == 2){
      std::cout << std::format("  movsx {}, word ptr [{}]\n", regs[d], regs[b]);
    } else if(size == 4){
      std::cout << std::format("  movsxd {}, dword ptr [{}]\n", regs[d], regs[b]);
    } else {
      assert(size == 8);
      std::cout << std::format("  mov {}, [{}]\n", regs[d], regs[b]);
    }
  }

  static void truncate(const std::shared_ptr<myLIR::LirNode>& lirNode){
    const int size = lirNode->type_size;
    const int a = lirNode->a ? lirNode->a->rn : 0;
    //signed trancate (type extension)
    if(size == 1){
      std::cout << std::format("  movsx {}, {}\n", regs[a], regs8[a]);
    } else if(size == 2){
      std::cout << std::format("  movsx {}, {}\n", regs[a], regs16[a]);
    } else if(size == 4){
      std::cout << std::format("  movsxd {}, {}\n", regs[a], regs32[a]);
    } else {
      assert(size == 8);
      std::cout << std::format("  mov {}, {}\n", regs[a], regs[a]);
    }
  }

static void gen(const std::shared_ptr<myLIR::LirNode>& lirNode){
  const int d = lirNode->d ? lirNode->d->rn : 0;
  const int a = lirNode->a ? lirNode->a->rn : 0;
  const int b = lirNode->b ? lirNode->b->rn : 0;

  switch(lirNode->opcode){
  case myLIR::LirKind::LIR_MOV:
    if(is_imm(lirNode->b)){
      if(is_int32(lirNode->b)){
	std::cout << std::format("  mov {}, {}\n", regs[d], lirNode->b->imm);
      } else {
	std::cout << std::format("  movabsq {}, {}\n", regs[d], lirNode->b->imm);
      }
    } else {
      if(d != b)
	std::cout << std::format("  mov {}, {}\n", regs[d], regs[b]);
    }
    break;
  case myLIR::LirKind::LIR_IMM:
    if(is_int32(lirNode)){
      std::cout << std::format("  mov {}, {}\n", regs[d], lirNode->imm);
    } else {
      std::cout << std::format("  movabsq {}, {}\n", regs[d], lirNode->imm);
    }
    break;
  case myLIR::LirKind::LIR_ADD:
    //64-bits immediate value is assumed to be stored in a register by movabsq before this instruction.
    // The instructions that can take 32-bits immediate value as its input are also assumed that.
    if(is_imm(lirNode->b) && is_int32(lirNode->b)){      
      std::cout << std::format("  add {}, {}\n", regs[d], lirNode->b->imm);
    } else {
      std::cout << std::format("  add {}, {}\n", regs[d], regs[b]);
    }
    break;
  case myLIR::LirKind::LIR_SUB:
    if(is_imm(lirNode->b) && is_int32(lirNode->b)){
      std::cout << std::format("  sub {}, {}\n", regs[d], lirNode->b->imm);
    } else {
      std::cout << std::format("  sub {}, {}\n", regs[d], regs[b]);
    }
    break;
  case myLIR::LirKind::LIR_MUL:
    /*
    std::cout << "  mov rax, " << regs[b] << std::endl;
    std::cout << "  imul " << regs[d] << std::endl;
    std::cout << "  mov " << regs[d] << ", rax" << std::endl;
    */
    //for 64 bit
    if(is_imm(lirNode->b) && is_int32(lirNode->b)){      
      std::cout << std::format("  imul {}, {}, {}\n", regs[d], regs[d], lirNode->b->imm);
    } else {
      //In this style, discard the upper 64 bits of the result
      std::cout << std::format("  imul {}, {}\n", regs[d], regs[b]);
    }
    //TODO: for 128 bit
    break;
  case myLIR::LirKind::LIR_MULHIGH:    
    std::cout << std::format("  mov rax, {}\n", regs[b]);
    std::cout << std::format("  imul {}\n", regs[d]);
    std::cout << std::format("  mov {}, rdx\n", regs[d]);
    break;
  case myLIR::LirKind::LIR_DIV:
    std::cout << "  mov rax, " << regs[d] << '\n';    
    std::cout << "  cqo\n";
    std::cout << "  idiv " << regs[b] << '\n';    
    std::cout << "  mov " << regs[d] << ", rax\n";
    break;
  case myLIR::LirKind::LIR_REM:
    std::cout << "  mov rax, " << regs[d] << '\n';
    std::cout << "  cqo\n";
    std::cout << "  idiv " << regs[b] << '\n';
    std::cout << "  mov " << regs[d] << ", rdx\n";
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
  case myLIR::LirKind::LIR_SHL:
    if(is_imm(lirNode->b) && is_int32(lirNode->b)){
      std::cout << std::format("  mov cl, {}\n", lirNode->b->imm);
    } else {
      std::cout << std::format("  mov cl, {}\n", regs8[b]);
    }
    std::cout << std::format("  shl {}, cl\n", regs[d]);
    break;
  case myLIR::LirKind::LIR_SHR:
    if(is_imm(lirNode->b) && is_int32(lirNode->b)){
      std::cout << std::format("  mov cl, {}\n", lirNode->b->imm);
    } else {
      std::cout << std::format("  mov cl, {}\n", regs8[b]);
    }
    std::cout << std::format("  shr {}, cl\n", regs[d]);
    break;
  case myLIR::LirKind::LIR_SAR:
    if(is_imm(lirNode->b) && is_int32(lirNode->b)){
      std::cout << std::format("  mov cl, {}\n", lirNode->b->imm);
    } else {
      std::cout << std::format("  mov cl, {}\n", regs8[b]);
    }
    std::cout << std::format("  sar {}, cl\n", regs[d]);
    break;
  case myLIR::LirKind::LIR_LVAR:
    std::cout << std::format("  lea {}, [rbp-{}]\n", regs[d], lirNode->lvar->offset);
    break;
  case myLIR::LirKind::LIR_LABEL_ADDR:
    std::cout << std::format("  lea {}, {}\n", regs[d], lirNode->name);
    break;
  case myLIR::LirKind::LIR_LOAD:
    load(lirNode);
    break;
  case myLIR::LirKind::LIR_LOAD_SPILL:
    std::cout << std::format("  mov {}, [rbp-{}]\n", regs[d], lirNode->lvar->offset);
    break;
  case myLIR::LirKind::LIR_STORE:
    if(is_imm(lirNode->b) && is_int32(lirNode->b)){
      const auto q = get_size_qualifier(lirNode->type_size);
      std::cout << std::format("  mov {} [{}], {}\n", q, regs[a], lirNode->b->imm);
    } else {
      std::cout << std::format("  mov [{}], {}\n", regs[a], reg(b, lirNode->type_size));
    }
    break;
  case myLIR::LirKind::LIR_STORE_SPILL:
    std::cout << std::format("  mov [rbp-{}], {}\n", lirNode->lvar->offset, regs[a]);
    break;
  case myLIR::LirKind::LIR_STORE_ARG:    
    std::cout << std::format("  mov [rbp-{}], {}\n",
			     lirNode->lvar->offset,
			     argreg(lirNode->imm, lirNode->type_size));
    break;
  case myLIR::LirKind::LIR_RETURN:
    if(lirNode->a){
      if(is_imm(lirNode->a) && is_int32(lirNode->a)){
	std::cout << std::format("  mov rax, {}\n", lirNode->a->imm);
      } else {
	std::cout << std::format("  mov rax, {}\n", regs[a]);
      }
    } //if(lirNode->a)
    std::cout << std::format("  jmp .L.return.{}\n", funcname);    
    break;
  case myLIR::LirKind::LIR_BR:
    if(is_imm(lirNode->b)){
      if(lirNode->b->imm == 0){
	std::cout << std::format("  jmp .L{}\n", lirNode->bb2->label);
      } else {
	std::cout << std::format("  jmp .L{}\n", lirNode->bb1->label);
      }
    } else {      
      std::cout << std::format("  test {}, {}\n", regs[b], regs[b]);
      std::cout << std::format("  je .L{}\n", lirNode->bb2->label);
      std::cout << std::format("  jmp .L{}\n", lirNode->bb1->label);
    }
    break;
  case myLIR::LirKind::LIR_JMP:
    if(lirNode->bbarg){
      if(is_imm(lirNode->bbarg)){
	if(is_int32(lirNode->bbarg)){
	  std::cout << std::format("  mov {}, {}\n", regs[lirNode->bb1->param->rn], lirNode->bbarg->imm);
	} else {
	  std::cout << std::format("  movabsq {}, {}\n", regs[lirNode->bb1->param->rn], lirNode->bbarg->imm);
	}
      } else {	
	std::cout << std::format("  mov {}, {}\n", regs[lirNode->bb1->param->rn], regs[lirNode->bbarg->rn]);
      }
    } //if(lirNode->bbarg)
    std::cout << std::format("  jmp .L{}\n", lirNode->bb1->label);
    break;
  case myLIR::LirKind::LIR_FUNCALL:
    for(int i = 0; i < lirNode->args.size(); i++){
      if(is_imm(lirNode->args[i]) && is_int32(lirNode->args[i])){
	std::cout << std::format("  mov {}, {}\n", argregs[i], lirNode->args[i]->imm);
      } else {	
	std::cout << std::format("  mov {}, {}\n", argregs[i], regs[lirNode->args[i]->rn]);
      }
    }
    std::cout << "  push r10\n" 
	      << "  push r11\n" 
	      << "  xor rax, rax\n"
	      << "  call " << lirNode->funcName << '\n'
	      << "  pop r11\n"
	      << "  pop r10\n"
	      << "  mov " << regs[d] << ", rax\n";
    break;
  case myLIR::LirKind::LIR_PTR_ADD: {
    /*
    std::cout << "  imul " << regs[b] << ", " << lirNode->type_base_size << std::endl;
    std::cout << "  add " << regs[d] << ", " << regs[b] << std::endl;
    */
    const int size = lirNode->type_base_size;
    if(size == 1){
      if(is_imm(lirNode->b) && is_int32(lirNode->b)){
	std::cout << std::format("  add {}, {}\n", regs[d], lirNode->b->imm);
      } else {
	std::cout << std::format("  add {}, {}\n", regs[d], regs[b]);
      }
    } else if(size == 2 || size == 4 || size == 8){
      if(is_imm(lirNode->b) && is_int32(lirNode->b)){
	std::cout << std::format("  add {}, {}\n", regs[d], lirNode->b->imm * size);
      } else {
	std::cout << std::format("  lea {}, [{}+{}*{}]\n",
				 regs[d], regs[d], regs[b], size);
      }
    } else {
      if(is_imm(lirNode->b) && is_int32(lirNode->b)){
	std::cout << std::format("  add {}, {}\n", regs[d], lirNode->b->imm * lirNode->type_base_size);
      } else {
	std::cout << std::format("  imul {}, {}\n", regs[b], lirNode->type_base_size);
	std::cout << std::format("  add {}, {}\n", regs[d], regs[b]);
      }
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
      if(is_imm(lirNode->b) && is_int32(lirNode->b)){
	std::cout << std::format("  sub {}, {}\n", regs[d], lirNode->b->imm);
      } else {
	std::cout << std::format("  sub {}, {}\n", regs[d], regs[b]);
      }
    } else if(size == 2 || size == 4 || size == 8){
      if(is_imm(lirNode->b) && is_int32(lirNode->b)){
	std::cout << std::format("  sub {}, {}\n", regs[d], lirNode->b->imm * size);
      } else {
	std::cout << std::format("  neg {}\n", regs[b]);
	std::cout << std::format("  lea {}, [{}+{}*{}]\n",
				 regs[d], regs[d], regs[b], size);
      }
    } else {
      if(is_imm(lirNode->b) && is_int32(lirNode->b)){
	std::cout << std::format("  sub {}, {}\n", regs[d], lirNode->b->imm * lirNode->type_base_size);
      } else {
	std::cout << std::format("  imul {}, {}\n", regs[b], lirNode->type_base_size);
	std::cout << std::format("  sub {}, {}\n", regs[d], regs[b]);
      }
    }
    break;
  }
  case myLIR::LirKind::LIR_PTR_DIFF: {
    const int size = lirNode->type_base_size;
    std::cout << std::format("  sub {}, {}\n", regs[d], regs[b]);
    if(size == 1){
      ;
    } else if(size == 2){
      std::cout << std::format("  sar {}, 1\n", regs[d]);
    } else if(size == 4){
      std::cout << std::format("  sar {}, 2\n", regs[d]);
    } else if(size == 8){
      std::cout << std::format("  sar {}, 3\n", regs[d]);
    } else {
      std::cout << std::format("  mov rax, {}\n", regs[d]);
      std::cout << "  cqo\n";
      std::cout << std::format("  mov {}, {}\n", regs[b], lirNode->type_base_size);
      std::cout << std::format("  idiv {}\n", regs[b]);
      std::cout << std::format("  mov {}, rax\n", regs[d]);
    }
    break;
  }
  case myLIR::LirKind::LIR_BITOR: {
    if(is_imm(lirNode->b) && is_int32(lirNode->b)){
      std::cout << std::format("  or {}, {}\n", regs[d], lirNode->b->imm);
    } else {
      std::cout << std::format("  or {}, {}\n", regs[d], regs[b]);
    }
    break;
  }
  case myLIR::LirKind::LIR_BITXOR: {
    if(is_imm(lirNode->b) && is_int32(lirNode->b)){
      std::cout << std::format("  xor {}, {}\n", regs[d], lirNode->b->imm);
    } else {
      std::cout << std::format("  xor {}, {}\n", regs[d], regs[b]);
    }
    break;
  }
  case myLIR::LirKind::LIR_BITAND: {
    if(is_imm(lirNode->b) && is_int32(lirNode->b)){
      std::cout << std::format("  and {}, {}\n", regs[d], lirNode->b->imm);
    } else {
      std::cout << std::format("  and {}, {}\n", regs[d], regs[b]);
    }
    break;
  }
  case myLIR::LirKind::LIR_CAST: {
    truncate(lirNode);
    break;
  }
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
    std::cout << ".bss\n";

    for(const auto& gvar: prog->globalVars){
      if(gvar->isLiteral || !gvar->initializer.empty()){
	continue;
      }      
      std::cout << std::format(".align {}\n", gvar->type->align);
      std::cout << std::format("{}:\n", gvar->name);
      std::cout << std::format("  .zero {}\n", gvar->type->size);
    }

    std::cout << ".data\n";
    for(const auto& gvar: prog->globalVars){
      if(gvar->isLiteral){	
	std::cout << std::format(".align {}\n", gvar->type->align);
	std::cout << std::format("{}:\n", gvar->name);
	print_literal(gvar->literal);
      }

      if(!gvar->initializer.empty()){	
	std::cout << std::format(".align {}\n", gvar->type->align);
	std::cout << std::format("{}:\n", gvar->name);
	for(const auto& init: gvar->initializer){
	  if(!init->label.empty()){
	    std::cout << std::format("  .quad {}{:+}\n", init->label, init->addend);
	  }
	  else if(init->size == 1){
	    std::cout << std::format("  .byte {}\n", init->val);
	  } else {	    
	    std::cout << std::format("  .{}byte {}\n", init->size, init->val);
	  }
	} //for init
      } //if !gvar->initializer.empty()
    } //for [name, gvar]
  }
  
static void emit_text(const std::unique_ptr<myLIR::Program>& prog){
  std::cout << ".text\n";

  for(const auto& fn: prog->fns){    
    std::cout << std::format(".global {}\n{}:\n", fn->name, fn->name);
    funcname = fn->name;

    //calculate stack size
    int offset = 0;
    for(const auto& lvar: fn->localVars){
      offset = Lunaria::align_to(offset, lvar->type->align);
      offset += lvar->type->size;
      lvar->offset = offset;
    }
    //16 byte alignment
    fn->stackSize = Lunaria::align_to(offset, 16);
    fn->stackSize += 8;
    
    //prologue
    std::cout << "  push rbp\n"
	      << "  mov rbp, rsp\n"
	      << "  sub rsp, " << fn->stackSize << '\n';
    std::cout << "  push r12\n"
	      << "  push r13\n"
	      << "  push r14\n"
	      << "  push r15\n"
	      << "  push rbx\n";

    //generate code
    for(const auto& bb: fn->bbs){
      std::cout << std::format(".L{}:\n", bb->label);
      for(const auto& lirNode: bb->insts){
	gen(lirNode);
      }
    }
    
    //epilogue
    std::cout << ".L.return." << funcname << ":\n";
    std::cout << "  pop rbx\n"
	      << "  pop r15\n"
	      << "  pop r14\n"
	      << "  pop r13\n"
	      << "  pop r12\n";
    std::cout << "  mov rsp, rbp\n"
    //std::cout << "  add rsp, " << fn->stackSize << std::endl
	      << "  pop rbp\n"
	      << "  ret\n";
  } //for fn
}

void gen_x86_64(const std::unique_ptr<myLIR::Program>& prog){
  std::cout << ".intel_syntax noprefix\n";
  emit_data(prog);
  emit_text(prog);
}

} //namespace myCodeGen
