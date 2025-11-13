#include "ycc.hpp"

namespace myHIR {
std::shared_ptr<HirNode>
new_node(HirKind kind){
  auto node = std::make_shared<HirNode>();
  node->kind = kind;
  return node;
}

std::shared_ptr<HirNode>
new_binary(HirKind kind,
	   std::shared_ptr<HirNode>& lhs,
	   std::shared_ptr<HirNode>& rhs){
  auto hirNode = new_node(kind);
  /*
  hirNode->lhs = std::move(lhs);
  hirNode->rhs = std::move(rhs);
  */
  hirNode->lhs = lhs;
  hirNode->rhs = rhs;
  return hirNode;
}

static std::shared_ptr<HirNode>
new_num(const std::unique_ptr<myParser::AstNode>& astNode){
  auto hirNode = new_node(HirKind::HIR_IMM);
  hirNode->val = astNode->val;
  hirNode->type = Lunaria::int_type;
  return hirNode;
}

std::shared_ptr<HirNode>
new_num(long long i){
  auto hirNode = new_node(HirKind::HIR_IMM);
  hirNode->val = i;
  hirNode->type = Lunaria::int_type;
  return hirNode;
}

static std::shared_ptr<HirNode>
new_var(const std::unique_ptr<myParser::AstNode>& astNode){
  auto hirNode = new_node(HirKind::HIR_VAR);
  hirNode->type = astNode->var->type;
  hirNode->var = std::move(astNode->var);
  return hirNode;
}

std::shared_ptr<HirNode>
new_var_node(const std::shared_ptr<Lunaria::Var>& var){
  auto hirNode = new_node(HirKind::HIR_VAR);
  hirNode->type = var->type;
  hirNode->var = var;
  return hirNode;
}

std::shared_ptr<HirNode>
new_add(std::shared_ptr<HirNode>& lhs,
	std::shared_ptr<HirNode>& rhs){
  if(is_integer(lhs->type) && is_integer(rhs->type)){
      //lhs:int rhs:int
      return new_binary(HirKind::HIR_ADD, lhs, rhs);
    }
    if(lhs->type->base && is_integer(rhs->type)){
      //lhs:pointer rhs:int
      return new_binary(HirKind::HIR_PTR_ADD, lhs, rhs);
    }
    if(is_integer(lhs->type) && rhs->type->base){
      //lhs:int rhs:pointer
      return new_binary(HirKind::HIR_PTR_ADD, rhs, lhs);
    }
    std::cerr << "invalid operands" << std::endl;
    exit(1);
}

std::shared_ptr<HirNode>
new_sub(std::shared_ptr<HirNode>& lhs,
	std::shared_ptr<HirNode>& rhs){
  if(is_integer(lhs->type) && is_integer(rhs->type)){
      //lhs:int rhs:int
      return new_binary(HirKind::HIR_SUB, lhs, rhs);
    }
    if(lhs->type->base && is_integer(rhs->type)){
      //lhs:pointer rhs:int
      return new_binary(HirKind::HIR_PTR_SUB, lhs, rhs);
    }
    if(lhs->type->base && rhs->type->base){
      //lhs:pointer rhs:pointer
      return new_binary(HirKind::HIR_PTR_DIFF, lhs, rhs);
    }
    std::cerr << "invalid operands" << std::endl;
    exit(1);
}

std::shared_ptr<HirNode>
program(const std::unique_ptr<myParser::AstNode>& astNode){
  if(astNode->kind == myParser::AstKind::AST_NUM){
    return new_num(astNode);
  } else if(astNode->kind == myParser::AstKind::AST_NULL){
    return new_node(HirKind::HIR_NULL);
  } else if(astNode->kind == myParser::AstKind::AST_VAR){
    return new_var(astNode);
  } else if(astNode->kind == myParser::AstKind::AST_RETURN){
    auto hirNode = new_node(HirKind::HIR_RETURN);
    if(astNode->lhs){
      auto lhs = program(astNode->lhs);
      hirNode->lhs = std::move(lhs);
      return hirNode;
    }
    hirNode->lhs = nullptr;
    return hirNode;
  }
  else if(astNode->kind == myParser::AstKind::AST_DEREF){
    auto lhs = program(astNode->lhs);
    auto hirNode = new_node(HirKind::HIR_DEREF);
    if(!lhs->type->base){
      std::cerr << "invalid pointer reference" << std::endl;
      exit(1);
    }
    hirNode->type = lhs->type->base;
    hirNode->lhs = std::move(lhs);
    return hirNode;
  }
  else if(astNode->kind == myParser::AstKind::AST_ADDR){
    auto lhs = program(astNode->lhs);
    auto hirNode = new_node(HirKind::HIR_ADDR);
    if(lhs->type->kind == Lunaria::TypeKind::ARRAY){
      hirNode->type = Lunaria::pointer_to(lhs->type->base);
    } else {
      hirNode->type = Lunaria::pointer_to(lhs->type);
    }
    hirNode->lhs = std::move(lhs);
    return hirNode;
  }
  else if(astNode->kind == myParser::AstKind::AST_PRE_INC){
    auto lhs = program(astNode->lhs);
    auto hirNode = new_node(HirKind::HIR_PRE_INC);
    hirNode->lhs = std::move(lhs);
    hirNode->type = hirNode->lhs->type;
    return hirNode;
  }
  else if(astNode->kind == myParser::AstKind::AST_PRE_DEC){
    auto lhs = program(astNode->lhs);
    auto hirNode = new_node(HirKind::HIR_PRE_DEC);
    hirNode->lhs = std::move(lhs);
    hirNode->type = hirNode->lhs->type;
    return hirNode;
  }
  else if(astNode->kind == myParser::AstKind::AST_POST_INC){
    auto lhs = program(astNode->lhs);
    auto hirNode = new_node(HirKind::HIR_POST_INC);
    hirNode->lhs = std::move(lhs);
    hirNode->type = hirNode->lhs->type;
    return hirNode;
  }
  else if(astNode->kind == myParser::AstKind::AST_POST_DEC){
    auto lhs = program(astNode->lhs);
    auto hirNode = new_node(HirKind::HIR_POST_DEC);
    hirNode->lhs = std::move(lhs);
    hirNode->type = hirNode->lhs->type;
    return hirNode;
  }
  else if(astNode->kind == myParser::AstKind::AST_BLOCK){
    auto hirNode = new_node(HirKind::HIR_BLOCK);
    for(const auto& n: astNode->body){
      hirNode->body.push_back(program(n));
    }
    return hirNode;
  } else if(astNode->kind == myParser::AstKind::AST_FUNCALL){
    auto hirNode = new_node(HirKind::HIR_FUNCALL);
    hirNode->funcName = astNode->funcName;
    for(const auto& an: astNode->args){
      hirNode->args.push_back(program(an));
    }
    return hirNode;
  } else if(astNode->kind == myParser::AstKind::AST_IF){
    auto hirNode = new_node(HirKind::HIR_IF);
    auto cond = program(astNode->cond);
    auto then = program(astNode->then);
    std::shared_ptr<HirNode> els = nullptr;
    if(astNode->els){
      els = program(astNode->els);
    }
    hirNode->cond = std::move(cond);
    hirNode->then = std::move(then);
    hirNode->els = std::move(els);
    return hirNode;
  } else if(astNode->kind == myParser::AstKind::AST_WHILE){
    auto hirNode = new_node(HirKind::HIR_WHILE);
    auto cond = program(astNode->cond);
    auto then = program(astNode->then);
    hirNode->cond = std::move(cond);
    hirNode->then = std::move(then);
    return hirNode;
  } else if(astNode->kind == myParser::AstKind::AST_FOR){
    auto hirNode = new_node(HirKind::HIR_FOR);
    std::shared_ptr<HirNode> init = nullptr;
    std::shared_ptr<HirNode> cond = nullptr;
    std::shared_ptr<HirNode> inc = nullptr;
    if(astNode->init){
      init = program(astNode->init);
    }
    if(astNode->cond){
      cond = program(astNode->cond);
    }
    if(astNode->inc){
      inc = program(astNode->inc);
    }

    auto then = program(astNode->then);
    hirNode->init = std::move(init);
    hirNode->cond = std::move(cond);
    hirNode->inc = std::move(inc);
    hirNode->then = std::move(then);
    return hirNode;
  } else {
    auto lhs = program(astNode->lhs);
    auto rhs = program(astNode->rhs);
    switch(astNode->kind){
    case myParser::AstKind::AST_ADD: {
      auto node = new_binary(HirKind::HIR_ADD, lhs, rhs);
      node->type = Lunaria::int_type;
      return node;
    }
    case myParser::AstKind::AST_SUB: {
      auto node = new_binary(HirKind::HIR_SUB, lhs, rhs);
      node->type = Lunaria::int_type;
      return node;
    }
    case myParser::AstKind::AST_MUL: {
      auto node = new_binary(HirKind::HIR_MUL, lhs, rhs);
      node->type = Lunaria::int_type;
      return node;
    }
    case myParser::AstKind::AST_DIV: {
      auto node = new_binary(HirKind::HIR_DIV, lhs, rhs);
      node->type = Lunaria::int_type;
      return node;
    }
    case myParser::AstKind::AST_LT: {
      auto node = new_binary(HirKind::HIR_LT, lhs, rhs);
      node->type = Lunaria::int_type;
      return node;
    }
    case myParser::AstKind::AST_LE: {
      auto node = new_binary(HirKind::HIR_LE, lhs, rhs);
      node->type = Lunaria::int_type;
      return node;
    }
    case myParser::AstKind::AST_EQ: {
      auto node = new_binary(HirKind::HIR_EQ, lhs, rhs);
      node->type = Lunaria::int_type;
      return node;
    }
    case myParser::AstKind::AST_NE: {
      auto node = new_binary(HirKind::HIR_NE, lhs, rhs);
      node->type = Lunaria::int_type;
      return node;
    }
    case myParser::AstKind::AST_ASSIGN: {
      auto node = new_binary(HirKind::HIR_ASSIGN, lhs, rhs);
      node->type = node->lhs->type;
      return node;
    }
    case myParser::AstKind::AST_PTR_ADD: {
      auto node = new_binary(HirKind::HIR_PTR_ADD, lhs, rhs);
      node->type = node->lhs->type;
      return node;
    }
    case myParser::AstKind::AST_PTR_SUB: {
      auto node = new_binary(HirKind::HIR_PTR_SUB, lhs, rhs);
      node->type = node->lhs->type;
      return node;
    }
    case myParser::AstKind::AST_PTR_DIFF: {
      auto node = new_binary(HirKind::HIR_PTR_DIFF, lhs, rhs);
      //node->type = Lunaria::int_type;
      node->type = node->lhs->type;
      return node;
    }
    case myParser::AstKind::AST_SUBSCRIPTED: {
      auto node = new_binary(HirKind::HIR_SUBSCRIPTED, lhs, rhs);
      node->type = node->lhs->type->base;
      return node;
    }
    } //switch
  }
  return nullptr;
}

std::unique_ptr<Program>
generateHirNode(const std::unique_ptr<myParser::Program>& prog){
  auto progHir = std::make_unique<Program>();
  progHir->globalVars = prog->globalVars;
  for(auto& fn: prog->fns){
    auto fnHir = std::make_unique<Function>();
    fnHir->name = fn->name;
    fnHir->params = fn->params;
    fnHir->localVars = fn->localVars;
    for(auto& astNode: fn->body){
      fnHir->body.push_back(program(astNode));
    }
    progHir->fns.push_back(std::move(fnHir));
  }
  return progHir;
}
} //namespace myHIR
