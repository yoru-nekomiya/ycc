#include "ycc.hpp"

namespace myHIR {
std::unique_ptr<HirNode>
new_node(HirKind kind){
  auto node = std::make_unique<HirNode>();
  node->kind = kind;
  return node;
}

std::unique_ptr<HirNode>
new_binary(HirKind kind,
	   std::unique_ptr<HirNode>& lhs,
	   std::unique_ptr<HirNode>& rhs){
  auto hirNode = new_node(kind);
  hirNode->lhs = std::move(lhs);
  hirNode->rhs = std::move(rhs);
  return hirNode;
}

static std::unique_ptr<HirNode>
new_num(const std::unique_ptr<myParser::AstNode>& astNode){
  auto hirNode = new_node(HirKind::HIR_IMM);
  hirNode->val = astNode->val;
  hirNode->type = Lunaria::int_type;
  return hirNode;
}

static std::unique_ptr<HirNode>
new_lvar(const std::unique_ptr<myParser::AstNode>& astNode){
  auto hirNode = new_node(HirKind::HIR_LVAR);
  hirNode->type = astNode->lvar->type;
  hirNode->lvar = std::move(astNode->lvar);
  return hirNode;
}

std::unique_ptr<HirNode>
program(const std::unique_ptr<myParser::AstNode>& astNode){
  if(astNode->kind == myParser::AstKind::AST_NUM){
    return new_num(astNode);
  } else if(astNode->kind == myParser::AstKind::AST_NULL){
    return new_node(HirKind::HIR_NULL);
  } else if(astNode->kind == myParser::AstKind::AST_LVAR){
    return new_lvar(astNode);
  } else if(astNode->kind == myParser::AstKind::AST_RETURN){
    auto lhs = program(astNode->lhs);
    auto hirNode = new_node(HirKind::HIR_RETURN);
    hirNode->lhs = std::move(lhs);
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
    std::unique_ptr<HirNode> els = nullptr;
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
    std::unique_ptr<HirNode> init = nullptr;
    std::unique_ptr<HirNode> cond = nullptr;
    std::unique_ptr<HirNode> inc = nullptr;
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
      node->type = Lunaria::int_type;
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
