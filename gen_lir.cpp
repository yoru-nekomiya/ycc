#include "ycc.hpp"

static std::list<std::shared_ptr<LirNode>> lirList = {};
static int nreg = 1; //represents a virtual register number
static std::shared_ptr<BasicBlock> outBB = nullptr;
static int label = 0;
static std::list<std::shared_ptr<BasicBlock>> BBList = {};

static std::shared_ptr<LirNode>
new_lir(LirKind opcode){
  auto lirNode = std::make_shared<LirNode>();
  lirNode->opcode = opcode;
  //lirList.push_back(lirNode);
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
  BBList.push_back(bb);
  return bb;
}

static std::shared_ptr<LirNode>
load(const std::shared_ptr<LirNode>& dst,
     const std::shared_ptr<LirNode>& src){
  auto node = emit_lir(LirKind::LIR_LOAD, dst, nullptr, src);
  node->lvar = src->lvar;
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

static std::shared_ptr<LirNode> gen_lval_lir(const std::unique_ptr<HirNode>& hirNode){
  auto lirNode = new_lir(LirKind::LIR_LVAR);
  auto d = new_reg(hirNode->lvar->name);
  lirNode->vn = d->vn;
  lirNode->d = std::move(d);
  lirNode->lvar = std::move(hirNode->lvar);
  return lirNode->d;
}

std::shared_ptr<LirNode> gen_expr_lir(const std::unique_ptr<HirNode>& hirNode);

static std::shared_ptr<LirNode>
gen_binop_lir(LirKind opcode,
	      const std::unique_ptr<HirNode>& hirNode){
  auto d = new_reg();
  auto a = gen_expr_lir(hirNode->lhs);
  auto b = gen_expr_lir(hirNode->rhs);
  auto lirNode = emit_lir(opcode, d, a, b);
  return lirNode->d;
}

std::shared_ptr<LirNode>
gen_expr_lir(const std::unique_ptr<HirNode>& hirNode){
  switch(hirNode->kind){
  case HirKind::HIR_IMM:
    return new_imm(hirNode->val);
  case HirKind::HIR_ADD:
    return gen_binop_lir(LirKind::LIR_ADD, hirNode);
  case HirKind::HIR_SUB:
    return gen_binop_lir(LirKind::LIR_SUB, hirNode);
  case HirKind::HIR_MUL:
    return gen_binop_lir(LirKind::LIR_MUL, hirNode);
  case HirKind::HIR_DIV:
    return gen_binop_lir(LirKind::LIR_DIV, hirNode);
  case HirKind::HIR_LT:
    return gen_binop_lir(LirKind::LIR_LT, hirNode);
  case HirKind::HIR_LE:
    return gen_binop_lir(LirKind::LIR_LE, hirNode);
  case HirKind::HIR_EQ:
    return gen_binop_lir(LirKind::LIR_EQ, hirNode);
  case HirKind::HIR_NE:
    return gen_binop_lir(LirKind::LIR_NE, hirNode);
  case HirKind::HIR_LVAR: {
    auto reg = new_reg(hirNode->lvar->name);
    auto node_lval = gen_lval_lir(hirNode);
    auto lirNode = load(reg, node_lval);
    return lirNode;
  }
  case HirKind::HIR_ASSIGN: {
    auto a = gen_lval_lir(hirNode->lhs);
    auto b = gen_expr_lir(hirNode->rhs);
    auto lirNode = emit_lir(LirKind::LIR_STORE, nullptr, a, b);
    return std::move(a);
  }
  case HirKind::HIR_RETURN: {
    auto a = gen_expr_lir(hirNode->lhs);
    auto lirNode = new_lir(LirKind::LIR_RETURN);
    lirNode->a = a;
    outBB = new_bb();
    return std::move(a);
  }
  case HirKind::HIR_IF: {
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
  case HirKind::HIR_WHILE: {
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
  case HirKind::HIR_FOR: {
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
  case HirKind::HIR_BLOCK: {
    for(const auto& n: hirNode->body){
      gen_expr_lir(n);
    }
    return nullptr;
  }
  case HirKind::HIR_FUNCALL: {
    auto lirNode = new_lir(LirKind::LIR_FUNCALL);
    lirNode->d = new_reg();
    lirNode->funcName = hirNode->funcName;
    return lirNode->d;
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

std::list<std::shared_ptr<BasicBlock>>
generateLirNode(const std::list<std::unique_ptr<HirNode>>& hirNodeList){
  outBB = new_bb();
  for(const auto& hirNode: hirNodeList){
    gen_expr_lir(hirNode);
  }
  //dumpLIR(lirList);
  return std::move(BBList);
}
