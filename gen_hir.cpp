#include "ycc.hpp"

static std::unique_ptr<HirNode>
new_node(HirKind kind){
  auto node = std::make_unique<HirNode>();
  node->kind = kind;
  return node;
}

static std::unique_ptr<HirNode>
new_binary(HirKind kind,
	   std::unique_ptr<HirNode>& lhs,
	   std::unique_ptr<HirNode>& rhs){
  auto hirNode = new_node(kind);
  hirNode->lhs = std::move(lhs);
  hirNode->rhs = std::move(rhs);
  return hirNode;
}

static std::unique_ptr<HirNode>
new_num(const std::unique_ptr<AstNode>& astNode){
  auto hirNode = new_node(HirKind::HIR_IMM);
  hirNode->val = astNode->val;
  return hirNode;
}

static std::unique_ptr<HirNode>
new_lvar(const std::unique_ptr<AstNode>& astNode){
  auto hirNode = new_node(HirKind::HIR_LVAR);
  hirNode->lvar = std::move(astNode->lvar);
  return hirNode;
}

std::unique_ptr<HirNode>
program(const std::unique_ptr<AstNode>& astNode){
  if(astNode->kind == AstKind::AST_NUM){
    return new_num(astNode);
  } else if(astNode->kind == AstKind::AST_LVAR){
    return new_lvar(astNode);
  } else if(astNode->kind == AstKind::AST_RETURN){
    auto lhs = program(astNode->lhs);
    auto hirNode = new_node(HirKind::HIR_RETURN);
    hirNode->lhs = std::move(lhs);
    return hirNode;
  }else {
    auto lhs = program(astNode->lhs);
    auto rhs = program(astNode->rhs);
    switch(astNode->kind){
    case AstKind::AST_ADD:
      return new_binary(HirKind::HIR_ADD, lhs, rhs);
    case AstKind::AST_SUB:
      return new_binary(HirKind::HIR_SUB, lhs, rhs);
    case AstKind::AST_MUL:
      return new_binary(HirKind::HIR_MUL, lhs, rhs);
    case AstKind::AST_DIV:
      return new_binary(HirKind::HIR_DIV, lhs, rhs);
    case AstKind::AST_LT:
      return new_binary(HirKind::HIR_LT, lhs, rhs);
    case AstKind::AST_LE:
      return new_binary(HirKind::HIR_LE, lhs, rhs);
    case AstKind::AST_EQ:
      return new_binary(HirKind::HIR_EQ, lhs, rhs);
    case AstKind::AST_NE:
      return new_binary(HirKind::HIR_NE, lhs, rhs);
    case AstKind::AST_ASSIGN:
      return new_binary(HirKind::HIR_ASSIGN, lhs, rhs);
    }
  }
  return nullptr;
}

std::list<std::unique_ptr<HirNode>>
generateHirNode(const std::list<std::unique_ptr<AstNode>>& astNodeList){
  std::list<std::unique_ptr<HirNode>> hirNodeList;
  for(const auto& astNode: astNodeList){
    hirNodeList.push_back(program(astNode));
  }
  return hirNodeList;
}
