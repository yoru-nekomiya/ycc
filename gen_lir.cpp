#include "ycc.hpp"

namespace myLIR {
  static int nreg = 1; //represents a virtual register number
  static std::shared_ptr<Function> func = nullptr;
  static std::shared_ptr<BasicBlock> outBB = nullptr;
  static int label = 0;

static std::shared_ptr<LirNode>
new_lir(LirKind opcode){
  auto lirNode = std::make_shared<LirNode>();
  lirNode->opcode = opcode;
  outBB->insts.push_back(lirNode);
  return std::move(lirNode);
}

static std::shared_ptr<LirNode>
emit_lir(LirKind opcode,
	 const std::shared_ptr<LirNode>& d,
	 const std::shared_ptr<LirNode>& a,
	 const std::shared_ptr<LirNode>& b){
  auto lirNode = new_lir(opcode);
  lirNode->d = d;
  lirNode->a = a;
  lirNode->b = b;
  return std::move(lirNode);
}

static std::unordered_map<std::string, int> map_varName2nreg;
static std::shared_ptr<LirNode>
new_reg(const std::string& varName = ""){
  auto lirNode = std::make_shared<LirNode>();
  lirNode->opcode = LirKind::LIR_REG;
  if(varName == ""){
    lirNode->vn = nreg++;
  }
  else if(map_varName2nreg.contains(varName)){    
    lirNode->vn = map_varName2nreg[varName];
  } else {
    lirNode->vn = nreg++;
  }
  return std::move(lirNode);
}

static std::shared_ptr<LirNode>
new_imm(int imm){
  auto d = new_reg();
  auto lirNode = new_lir(LirKind::LIR_IMM);
  lirNode->vn = d->vn;
  lirNode->d = std::move(d);
  lirNode->imm = imm;
  return lirNode->d;
}

static std::shared_ptr<BasicBlock>
new_bb(){
  auto bb = std::make_shared<BasicBlock>();
  bb->label = label++;
  //BBList.push_back(bb);
  func->bbs.push_back(bb);
  return bb;
}

static std::shared_ptr<LirNode>
load(const std::shared_ptr<LirNode>& dst,
     const std::shared_ptr<LirNode>& src,
     int type_size){
  auto node = emit_lir(LirKind::LIR_LOAD, dst, nullptr, src);
  node->lvar = src->lvar;
  node->type_size = type_size;
  return node->d;
}

static std::shared_ptr<LirNode>
br(const std::shared_ptr<LirNode>& b,
   const std::shared_ptr<BasicBlock>& then,
   const std::shared_ptr<BasicBlock>& els){
  auto lirNode = new_lir(LirKind::LIR_BR);
  lirNode->b = b;
  lirNode->bb1 = then;
  lirNode->bb2 = els;
  return lirNode;
}

static std::shared_ptr<LirNode>
jmp(const std::shared_ptr<BasicBlock>& bb){
  auto lirNode = new_lir(LirKind::LIR_JMP);
  lirNode->bb1 = bb;
  return lirNode;
}

static std::shared_ptr<LirNode> gen_lval_lir(const std::unique_ptr<myHIR::HirNode>& hirNode){
  auto lirNode = new_lir(LirKind::LIR_LVAR);
  auto d = new_reg(hirNode->lvar->name);
  lirNode->vn = d->vn;
  lirNode->d = std::move(d);
  lirNode->lvar = std::move(hirNode->lvar);
  return lirNode->d;
}

std::shared_ptr<LirNode> gen_expr_lir(const std::unique_ptr<myHIR::HirNode>& hirNode);

static std::shared_ptr<LirNode>
gen_binop_lir(LirKind opcode,
	      const std::unique_ptr<myHIR::HirNode>& hirNode){
  auto d = new_reg();
  auto a = gen_expr_lir(hirNode->lhs);
  auto b = gen_expr_lir(hirNode->rhs);
  auto lirNode = emit_lir(opcode, d, a, b);
  lirNode->type_base_size = (hirNode->type->base) ? hirNode->type->base->size : 0;
  return lirNode->d;
}

std::shared_ptr<LirNode>
gen_expr_lir(const std::unique_ptr<myHIR::HirNode>& hirNode){
  switch(hirNode->kind){
  case myHIR::HirKind::HIR_IMM:
    return new_imm(hirNode->val);
  case myHIR::HirKind::HIR_NULL:
    return nullptr;
  case myHIR::HirKind::HIR_ADD:
    return gen_binop_lir(LirKind::LIR_ADD, hirNode);
  case myHIR::HirKind::HIR_SUB:
    return gen_binop_lir(LirKind::LIR_SUB, hirNode);
  case myHIR::HirKind::HIR_MUL:
    return gen_binop_lir(LirKind::LIR_MUL, hirNode);
  case myHIR::HirKind::HIR_DIV:
    return gen_binop_lir(LirKind::LIR_DIV, hirNode);
  case myHIR::HirKind::HIR_LT:
    return gen_binop_lir(LirKind::LIR_LT, hirNode);
  case myHIR::HirKind::HIR_LE:
    return gen_binop_lir(LirKind::LIR_LE, hirNode);
  case myHIR::HirKind::HIR_EQ:
    return gen_binop_lir(LirKind::LIR_EQ, hirNode);
  case myHIR::HirKind::HIR_NE:
    return gen_binop_lir(LirKind::LIR_NE, hirNode);
  case myHIR::HirKind::HIR_PTR_ADD:
    return gen_binop_lir(LirKind::LIR_PTR_ADD, hirNode);
  case myHIR::HirKind::HIR_PTR_SUB:
    return gen_binop_lir(LirKind::LIR_PTR_SUB, hirNode);
  case myHIR::HirKind::HIR_PTR_DIFF:
    return gen_binop_lir(LirKind::LIR_PTR_DIFF, hirNode);
  case myHIR::HirKind::HIR_LVAR: {
    auto reg = new_reg(hirNode->lvar->name);
    auto node_lval = gen_lval_lir(hirNode);
    auto lirNode = load(reg, node_lval, hirNode->type->size);
    return lirNode;
  }
  case myHIR::HirKind::HIR_ASSIGN: {
    auto a = gen_lval_lir(hirNode->lhs);
    auto b = gen_expr_lir(hirNode->rhs);
    auto lirNode = emit_lir(LirKind::LIR_STORE, nullptr, a, b);
    lirNode->type_size = hirNode->type->size;
    return std::move(a);
  }
  case myHIR::HirKind::HIR_RETURN: {
    auto a = gen_expr_lir(hirNode->lhs);
    auto lirNode = new_lir(LirKind::LIR_RETURN);
    lirNode->a = a;
    outBB = new_bb();
    return std::move(a);
  }
  case myHIR::HirKind::HIR_IF: {
    std::shared_ptr<BasicBlock> then = new_bb();
    std::shared_ptr<BasicBlock> els = new_bb();
    std::shared_ptr<BasicBlock> last = new_bb();

    auto node_cond = gen_expr_lir(hirNode->cond);    
    br(node_cond, then, els);

    outBB = then;
    gen_expr_lir(hirNode->then);
    jmp(last);

    outBB = els;
    if(hirNode->els){
      gen_expr_lir(hirNode->els);
    }
    jmp(last);

    outBB = last;
    return nullptr;
  }
  case myHIR::HirKind::HIR_WHILE: {
    std::shared_ptr<BasicBlock> cond = new_bb();
    std::shared_ptr<BasicBlock> body = new_bb();
    std::shared_ptr<BasicBlock> _break = new_bb();

    outBB = cond;
    auto node_cond = gen_expr_lir(hirNode->cond);
    br(node_cond, body, _break);

    outBB = body;
    gen_expr_lir(hirNode->then);
    jmp(cond);

    outBB = _break;
    return nullptr;
  }
  case myHIR::HirKind::HIR_FOR: {
    std::shared_ptr<BasicBlock> cond = new_bb();
    std::shared_ptr<BasicBlock> body = new_bb();
    std::shared_ptr<BasicBlock> _break = new_bb();

    if(hirNode->init){
      gen_expr_lir(hirNode->init);
    }
    jmp(cond);

    outBB = cond;
    if(hirNode->cond){
      auto node_cond = gen_expr_lir(hirNode->cond);
      br(node_cond, body, _break);
    } else {
      jmp(body);
    }

    outBB = body;
    gen_expr_lir(hirNode->then);

    if(hirNode->inc){
      gen_expr_lir(hirNode->inc);
    }
    jmp(cond);

    outBB = _break;
    return nullptr;
  }
  case myHIR::HirKind::HIR_BLOCK: {
    for(const auto& n: hirNode->body){
      gen_expr_lir(n);
    }
    return nullptr;
  }
  case myHIR::HirKind::HIR_FUNCALL: {
    std::vector<std::shared_ptr<LirNode>> args;
    for(const auto& n: hirNode->args){
      auto d = gen_expr_lir(n);
      args.push_back(d);
    }
    auto lirNode = new_lir(LirKind::LIR_FUNCALL);
    lirNode->d = new_reg();
    lirNode->funcName = hirNode->funcName;
    lirNode->args = std::move(args);
    return lirNode->d;
  }
  case myHIR::HirKind::HIR_DEREF: {
    auto reg = new_reg();
    auto lirNode = load(reg, gen_expr_lir(hirNode->lhs), hirNode->type->size);
    return lirNode;
  }
  case myHIR::HirKind::HIR_ADDR: {
    return gen_lval_lir(hirNode->lhs);
  }
  } //switch
  return nullptr;
}

void dumpLIR(const std::list<std::shared_ptr<LirNode>>& lirList){
  for(const auto& n: lirList){
    const int d = n->d ? n->d->vn : 0;
    const int a = n->a ? n->a->vn : 0;
    const int b = n->b ? n->b->vn : 0;

    switch(n->opcode){
    case LirKind::LIR_IMM:
      std::cout << "v" << d << " = " << n->imm << std::endl;
      break;
    case LirKind::LIR_MOV:
      std::cout << "v" << d << " = v" << b << std::endl;
      break;
    case LirKind::LIR_ADD:
      std::cout << "v" << d << " = v" << a << " + v" << b << std::endl;
      break;
    }
  } //for
}

  static void gen_param(std::shared_ptr<Lunaria::LVar> param,
		      unsigned int i){
  auto lirNode = new_lir(LirKind::LIR_STORE_ARG);
  lirNode->lvar = param;
  lirNode->imm = i;
  return;
}

std::unique_ptr<Program>
generateLirNode(const std::unique_ptr<myHIR::Program>& prog){
  auto progLir = std::make_unique<Program>();
  for(auto& fn: prog->fns){
    auto fnLir = std::make_shared<Function>();
    fnLir->name = fn->name;
    fnLir->params = fn->params;
    fnLir->localVars = fn->localVars;
    
    func = fnLir;
    outBB = new_bb();

    unsigned int i = 0;
    for(const auto& param: fn->params){
      gen_param(param, i++);
    }
    
    for(const auto& hirNode: fn->body){
      gen_expr_lir(hirNode);
    }
    progLir->fns.push_back(fnLir);
  }
  //dumpLIR(lirList);
  return progLir;
}
} //namespace myLIR
