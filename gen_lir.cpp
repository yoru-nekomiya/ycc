#include "ycc.hpp"

static std::list<std::shared_ptr<LirNode>> lirList = {};
static int nreg = 1; //represents a virtual register number

static std::shared_ptr<LirNode>
new_lir(LirKind opcode){
  /*
  auto lirNode = std::make_shared<LirNode>(opcode,
					   nullptr,
					   nullptr,
					   nullptr,
					   -1,
					   -1,
					   -1);
  */
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
  lirNode->d = std::move(d);
  lirNode->a = std::move(a);
  lirNode->b = std::move(b);
  return std::move(lirNode);
}

static std::shared_ptr<LirNode>
new_reg(){
  /*
  auto lirNode = std::make_shared<LirNode>(LirKind::LIR_REG,
					   nullptr,
					   nullptr,
					   nullptr,
					   -1,
					   nreg++,
					   -1);
  */
  auto lirNode = std::make_shared<LirNode>();
  lirNode->opcode = LirKind::LIR_REG;
  lirNode->vn = nreg++;
  return std::move(lirNode);
}

static std::shared_ptr<LirNode>
new_imm(int imm){
  auto d = new_reg();
  auto lirNode = new_lir(LirKind::LIR_IMM);
  lirNode->vn = d->vn;
  lirNode->d = std::move(d);
  lirNode->imm = imm;
  return std::move(lirNode);
}

std::shared_ptr<LirNode> gen_expr_lir(const std::unique_ptr<HirNode>& hirNode);

static std::shared_ptr<LirNode>
gen_binop_lir(LirKind opcode,
	      const std::unique_ptr<HirNode>& hirNode){
  auto d = new_reg();
  auto a = gen_expr_lir(hirNode->lhs);
  auto b = gen_expr_lir(hirNode->rhs);
  return emit_lir(opcode, d, a, b);
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
  }
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
generateLirNode(const std::unique_ptr<HirNode>& hirNode){
  auto lirNode = gen_expr_lir(hirNode);
  //dumpLIR(lirList);
  return std::move(lirList);
}
