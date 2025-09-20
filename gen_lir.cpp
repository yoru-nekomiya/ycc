#include "ycc.hpp"

static std::list<std::shared_ptr<LirNode>> lirList = {};
static int nreg = 1; //represents a virtual register number

static std::shared_ptr<LirNode>
new_lir(LirKind opcode){
  auto lirNode = std::make_shared<LirNode>();
  lirNode->opcode = opcode;
  lirList.push_back(lirNode);
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

static std::shared_ptr<LirNode>
load(const std::shared_ptr<LirNode>& dst,
     const std::shared_ptr<LirNode>& src){
  auto node = emit_lir(LirKind::LIR_LOAD, dst, nullptr, src);
  node->lvar = src->lvar;
  return node->d;
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
    return std::move(a);
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

std::list<std::shared_ptr<LirNode>>
generateLirNode(const std::list<std::unique_ptr<HirNode>>& hirNodeList){
  for(const auto& hirNode: hirNodeList){
    gen_expr_lir(hirNode);
  }
  //dumpLIR(lirList);
  return std::move(lirList);
}
