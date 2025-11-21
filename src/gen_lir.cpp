#include "ycc.hpp"

namespace myLIR {
  static int nreg = 1; //represents a virtual register number
  static std::shared_ptr<Function> func = nullptr;
  static std::shared_ptr<BasicBlock> outBB = nullptr;
  static int label = 0;
  static std::unordered_map<std::string, std::shared_ptr<Lunaria::Var>> localVars;

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
new_imm(long long imm){
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
  func->bbs.push_back(bb);
  return bb;
}

static std::shared_ptr<LirNode>
load(const std::shared_ptr<LirNode>& dst,
     const std::shared_ptr<LirNode>& src,
     int type_size){
  auto node = emit_lir(LirKind::LIR_LOAD, dst, nullptr, src);
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

  static std::shared_ptr<LirNode>
  jmp_arg(const std::shared_ptr<BasicBlock>& bb,
	  const std::shared_ptr<LirNode>& reg){
    auto lirNode = new_lir(LirKind::LIR_JMP);
    lirNode->bb1 = bb;
    lirNode->bbarg = reg;
    return lirNode;
  }

  std::shared_ptr<LirNode> gen_expr_lir(const std::shared_ptr<myHIR::HirNode>& hirNode);

static std::shared_ptr<LirNode> gen_lval_lir(const std::shared_ptr<myHIR::HirNode>& hirNode){
  if(hirNode->kind == myHIR::HirKind::HIR_DEREF){
    return gen_expr_lir(hirNode->lhs);
  }
  if(hirNode->kind == myHIR::HirKind::HIR_SUBSCRIPTED){
    //lhs in assign
    auto hirPtrAdd = myHIR::new_binary(myHIR::HirKind::HIR_PTR_ADD,
			       hirNode->lhs, hirNode->rhs);
    hirPtrAdd->type = hirPtrAdd->lhs->type;
    return gen_expr_lir(hirPtrAdd);
  }

  assert(hirNode->kind == myHIR::HirKind::HIR_VAR);
  std::shared_ptr<LirNode> lirNode = nullptr;
  if(hirNode->var->isLocal){
    lirNode = new_lir(LirKind::LIR_LVAR);
    auto d = new_reg(hirNode->var->name);
    lirNode->vn = d->vn;
    lirNode->d = std::move(d);
    lirNode->lvar = hirNode->var;
  } else {
    lirNode = new_lir(LirKind::LIR_LABEL_ADDR);
    auto d = new_reg(hirNode->var->name);
    lirNode->vn = d->vn;
    lirNode->d = std::move(d);
    lirNode->name = hirNode->var->name;
  }
  return lirNode->d;
}

static std::shared_ptr<LirNode>
gen_binop_lir(LirKind opcode,
	      const std::shared_ptr<myHIR::HirNode>& hirNode){
  auto d = new_reg();
  auto a = gen_expr_lir(hirNode->lhs);
  auto b = gen_expr_lir(hirNode->rhs);
  auto lirNode = emit_lir(opcode, d, a, b);
  lirNode->type_base_size = (hirNode->type->base) ? hirNode->type->base->size : 0;
  return lirNode->d;
}

  static std::shared_ptr<Lunaria::Var> new_lvar(const std::string& name, const std::shared_ptr<Lunaria::Type>& type){
    auto var = std::make_shared<Lunaria::Var>();
    var->name = name;
    var->type = type;
    var->isLocal = true;
    localVars[var->name] = var;
    return var;
  }

  static std::shared_ptr<LirNode>
  gen_op_assign(myHIR::HirKind k,
		const std::shared_ptr<myHIR::HirNode>& hirNode){
    //a op= b --> T* t = &a; *t = *t op b;
    //T* t = &a;
    static int i = 0;
    auto hirAssign_t = myHIR::new_node(myHIR::HirKind::HIR_ASSIGN);
    auto t = new_lvar("__tmp_var_opassign__" + std::to_string(i++), pointer_to(hirNode->lhs->type));
    hirAssign_t->lhs = myHIR::new_var_node(t);
    auto rhs_t = myHIR::new_node(myHIR::HirKind::HIR_ADDR);
    rhs_t->lhs = hirNode->lhs;
    hirAssign_t->rhs = rhs_t;
    myHIR::add_type(hirAssign_t);
    gen_expr_lir(hirAssign_t);
    
    //*t = *t op b;
    auto hirAssign_op = myHIR::new_node(myHIR::HirKind::HIR_ASSIGN);
    auto lhs_op = myHIR::new_node(myHIR::HirKind::HIR_DEREF);
    lhs_op->lhs = myHIR::new_var_node(t);
    hirAssign_op->lhs = lhs_op;
    
    auto op_node = myHIR::new_node(k);
    auto lhs_op_node = myHIR::new_node(myHIR::HirKind::HIR_DEREF);
    lhs_op_node->lhs = myHIR::new_var_node(t);
    op_node->lhs = lhs_op_node;
    op_node->rhs = hirNode->rhs;
    hirAssign_op->rhs = op_node;
    myHIR::add_type(hirAssign_op);
    auto rlt = gen_expr_lir(hirAssign_op);
    return rlt;
  }
  
std::shared_ptr<LirNode>
gen_expr_lir(const std::shared_ptr<myHIR::HirNode>& hirNode){
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
  case myHIR::HirKind::HIR_SHL:
    return gen_binop_lir(LirKind::LIR_SHL, hirNode);
  case myHIR::HirKind::HIR_SAR:
    return gen_binop_lir(LirKind::LIR_SAR, hirNode);
  case myHIR::HirKind::HIR_PRE_INC: {
    //++i -> i=i+1
    auto hirAssign = myHIR::new_node(myHIR::HirKind::HIR_ASSIGN);
    hirAssign->lhs = hirNode->lhs;
    auto num_node = myHIR::new_num(1);
    hirAssign->rhs = myHIR::new_add(hirNode->lhs,
				    num_node);
    myHIR::add_type(hirAssign);
    return gen_expr_lir(hirAssign);
  }
  case myHIR::HirKind::HIR_PRE_DEC: {
    //--i -> i=i-1
    auto hirAssign = myHIR::new_node(myHIR::HirKind::HIR_ASSIGN);
    hirAssign->lhs = hirNode->lhs;
    auto num_node = myHIR::new_num(1);
    hirAssign->rhs = myHIR::new_sub(hirNode->lhs,
				    num_node);
    myHIR::add_type(hirAssign);
    return gen_expr_lir(hirAssign);
  }
  case myHIR::HirKind::HIR_POST_INC: {
    //i++ -> t=i; i=i+1; t;
    //t=i
    static int i = 0;
    auto hirAssign_t = myHIR::new_node(myHIR::HirKind::HIR_ASSIGN);
    auto t = new_lvar("__tmp_var_postinc__" + std::to_string(i++), hirNode->lhs->type);
    hirAssign_t->lhs = myHIR::new_var_node(t);
    hirAssign_t->rhs = hirNode->lhs;
    myHIR::add_type(hirAssign_t);
    auto rlt = gen_expr_lir(hirAssign_t);

    //i=i+1
    auto hirAssign_i = myHIR::new_node(myHIR::HirKind::HIR_ASSIGN);
    hirAssign_i->lhs = hirNode->lhs;
    auto num_node = myHIR::new_num(1);
    hirAssign_i->rhs = myHIR::new_add(hirNode->lhs, num_node);
    myHIR::add_type(hirAssign_i);
    gen_expr_lir(hirAssign_i);

    //t;
    return rlt;
  }
  case myHIR::HirKind::HIR_POST_DEC: {
    //i-- -> t=i; i=i-1; t;
    //t=i
    static int i = 0;
    auto hirAssign_t = myHIR::new_node(myHIR::HirKind::HIR_ASSIGN);
    auto t = new_lvar("__tmp_var_postdec__" + std::to_string(i++), hirNode->lhs->type);
    hirAssign_t->lhs = myHIR::new_var_node(t);
    hirAssign_t->rhs = hirNode->lhs;
    myHIR::add_type(hirAssign_t);
    auto rlt = gen_expr_lir(hirAssign_t);

    //i=i-1
    auto hirAssign_i = myHIR::new_node(myHIR::HirKind::HIR_ASSIGN);
    hirAssign_i->lhs = hirNode->lhs;
    auto num_node = myHIR::new_num(1);
    hirAssign_i->rhs = myHIR::new_sub(hirNode->lhs, num_node);
    myHIR::add_type(hirAssign_i);
    gen_expr_lir(hirAssign_i);

    //t;
    return rlt;
  }
  case myHIR::HirKind::HIR_VAR: {
    if(hirNode->type->kind == Lunaria::TypeKind::ARRAY){
      return gen_lval_lir(hirNode);
    }    
    auto reg = new_reg(hirNode->var->name);
    auto node_lval = gen_lval_lir(hirNode);
    auto lirNode = load(reg, node_lval, hirNode->type->size);
    return lirNode;
  }
  case myHIR::HirKind::HIR_ASSIGN: {
    auto a = gen_lval_lir(hirNode->lhs);
    auto b = gen_expr_lir(hirNode->rhs);
    auto lirNode = emit_lir(LirKind::LIR_STORE, nullptr, a, b);
    lirNode->type_size = hirNode->type->size;
    auto reg = new_reg();
    return load(reg, a, hirNode->type->size);
  }
  case myHIR::HirKind::HIR_ADD_ASSIGN: {
    const myHIR::HirKind kind = hirNode->lhs->type->base ? myHIR::HirKind::HIR_PTR_ADD : myHIR::HirKind::HIR_ADD;
    return gen_op_assign(kind, hirNode);
  }
  case myHIR::HirKind::HIR_SUB_ASSIGN: {
    const myHIR::HirKind kind = hirNode->lhs->type->base ? myHIR::HirKind::HIR_PTR_SUB : myHIR::HirKind::HIR_SUB;
    return gen_op_assign(kind, hirNode);
  }
  case myHIR::HirKind::HIR_MUL_ASSIGN: {
    return gen_op_assign(myHIR::HirKind::HIR_MUL, hirNode);
  }
  case myHIR::HirKind::HIR_DIV_ASSIGN: {
    return gen_op_assign(myHIR::HirKind::HIR_DIV, hirNode);
  }
  case myHIR::HirKind::HIR_RETURN: {
    std::shared_ptr<LirNode> a = nullptr;
    if(hirNode->lhs){
      a = gen_expr_lir(hirNode->lhs);
    }
    auto lirNode = new_lir(LirKind::LIR_RETURN);
    lirNode->a = a;
    outBB = new_bb();
    return std::move(a);
  }
  case myHIR::HirKind::HIR_IF: {
    /*
    std::shared_ptr<BasicBlock> then = new_bb();
    std::shared_ptr<BasicBlock> els = new_bb();
    std::shared_ptr<BasicBlock> last = new_bb();
    */
    auto node_cond = gen_expr_lir(hirNode->cond);
    std::shared_ptr<BasicBlock> then = new_bb();
    std::shared_ptr<BasicBlock> els = new_bb();
    br(node_cond, then, els);

    outBB = then;
    gen_expr_lir(hirNode->then);
    std::shared_ptr<BasicBlock> last = new_bb();
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
    /*
    std::shared_ptr<BasicBlock> cond = new_bb();
    std::shared_ptr<BasicBlock> body = new_bb();
    std::shared_ptr<BasicBlock> _break = new_bb();
    */
    std::shared_ptr<BasicBlock> cond = new_bb();
    outBB = cond;
    auto node_cond = gen_expr_lir(hirNode->cond);
    std::shared_ptr<BasicBlock> body = new_bb();
    std::shared_ptr<BasicBlock> _break = new_bb();
    br(node_cond, body, _break);

    outBB = body;
    gen_expr_lir(hirNode->then);
    jmp(cond);

    outBB = _break;
    return nullptr;
  }
  case myHIR::HirKind::HIR_FOR: {
    /*
    std::shared_ptr<BasicBlock> cond = new_bb();
    std::shared_ptr<BasicBlock> body = new_bb();
    std::shared_ptr<BasicBlock> _break = new_bb();
    */
    if(hirNode->init){
      gen_expr_lir(hirNode->init);
    }
    std::shared_ptr<BasicBlock> cond = new_bb();
    jmp(cond);

    outBB = cond;
    std::shared_ptr<BasicBlock> body = new_bb();
    std::shared_ptr<BasicBlock> _break = new_bb();
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
    if(hirNode->type->kind == Lunaria::TypeKind::ARRAY){
      return gen_expr_lir(hirNode->lhs);
    }
    auto reg = new_reg();
    auto lirNode = load(reg, gen_expr_lir(hirNode->lhs), hirNode->type->size);
    return lirNode;
  }
  case myHIR::HirKind::HIR_ADDR: {
    return gen_lval_lir(hirNode->lhs);
  }
  case myHIR::HirKind::HIR_SUBSCRIPTED: {
    //rhs in assign
    //a[i] -> *(a+i)
    auto hirPtrAdd = myHIR::new_binary(myHIR::HirKind::HIR_PTR_ADD,
			       hirNode->lhs, hirNode->rhs);
    hirPtrAdd->type = hirPtrAdd->lhs->type;
    auto hirDeref = myHIR::new_node(myHIR::HirKind::HIR_DEREF);
    //hirDeref->lhs = std::move(hirPtrAdd);
    hirDeref->lhs = hirPtrAdd;
    hirDeref->type = hirDeref->lhs->type->base;
    return gen_expr_lir(hirDeref);
  }
  case myHIR::HirKind::HIR_LOGOR: {
    const auto r1 = gen_expr_lir(hirNode->lhs);
    std::shared_ptr<BasicBlock> bb = new_bb();
    std::shared_ptr<BasicBlock> set1 = new_bb();
    br(r1, set1, bb);

    outBB = bb;
    const auto r2 = gen_expr_lir(hirNode->rhs);
    std::shared_ptr<BasicBlock> set0 = new_bb();
    br(r2, set1, set0);

    outBB = set0;
    const auto zero = new_imm(0);
    std::shared_ptr<BasicBlock> last = new_bb();
    jmp_arg(last, zero);

    outBB = set1;
    const auto one = new_imm(1);
    jmp_arg(last, one);

    outBB = last;
    outBB->param = new_reg();
    return outBB->param;
  }
  case myHIR::HirKind::HIR_LOGAND: {
    const auto r1 = gen_expr_lir(hirNode->lhs);
    std::shared_ptr<BasicBlock> bb = new_bb();
    std::shared_ptr<BasicBlock> set0 = new_bb();
    br(r1, bb, set0);

    outBB = bb;
    const auto r2 = gen_expr_lir(hirNode->rhs);
    std::shared_ptr<BasicBlock> set1 = new_bb();
    br(r2, set1, set0);

    outBB = set0;
    const auto zero = new_imm(0);
    std::shared_ptr<BasicBlock> last = new_bb();
    jmp_arg(last, zero);

    outBB = set1;
    const auto one = new_imm(1);
    jmp_arg(last, one);

    outBB = last;
    outBB->param = new_reg();
    return outBB->param;
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

  static void gen_param(std::shared_ptr<Lunaria::Var> param,
			unsigned int i){
    auto lirNode = new_lir(LirKind::LIR_STORE_ARG);
    lirNode->lvar = param;
    lirNode->imm = i;
    lirNode->type_size = param->type->size;
    return;
  }

std::unique_ptr<Program>
generateLirNode(const std::unique_ptr<myHIR::Program>& prog){
  auto progLir = std::make_unique<Program>();
  progLir->globalVars = prog->globalVars;
  
  for(auto& fn: prog->fns){
    auto fnLir = std::make_shared<Function>();
    fnLir->name = fn->name;
    fnLir->params = fn->params;
    //fnLir->localVars = fn->localVars;
    localVars = fn->localVars;
    
    func = fnLir;
    outBB = new_bb();

    unsigned int i = 0;
    for(const auto& param: fn->params){
      gen_param(param, i++);
    }
    
    for(const auto& hirNode: fn->body){
      gen_expr_lir(hirNode);
    }
    fnLir->localVars = localVars;
    progLir->fns.push_back(fnLir);
  }
  //dumpLIR(lirList);
  return progLir;
}
} //namespace myLIR
