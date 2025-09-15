#include "ycc.hpp"

static std::unique_ptr<HirNode>
new_node(HirKind kind){
  //return std::make_unique<HirNode>(kind, nullptr, nullptr, 0);
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
/*
static std::unique_ptr<HirNode> add(const std::unique_ptr<AstNode>& astNode);
static std::unique_ptr<HirNode> mul(const std::unique_ptr<AstNode>& astNode);
static std::unique_ptr<HirNode> unary(const std::unique_ptr<AstNode>& astNode);
static std::unique_ptr<HirNode> primary(const std::unique_ptr<AstNode>& astNode);
*/

std::unique_ptr<HirNode>
program(const std::unique_ptr<AstNode>& astNode){
  if(astNode->kind == AstKind::AST_NUM){
    return new_num(astNode);
  } else {
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
    }
  }
  return nullptr;
}
/*
//program = add
std::unique_ptr<HirNode>
program(const std::unique_ptr<AstNode>& astNode){
  return add(astNode);
}

//add = mul ("+" mul | "-" mul)*
static std::unique_ptr<HirNode>
add(const std::unique_ptr<AstNode>& astNode){
  auto node = mul(astNode->lhs);

  while(1){
    if(astNode->kind == AstKind::ND_ADD){
      auto node_mul = mul(astNode->rhs);
      node = new_binary(HirKind::HIR_ADD, node, node_mul);
    }
    else if(astNode->kind == AstKind::ND_SUB){
      auto node_mul = mul(astNode->rhs);
      node = new_binary(HirKind::HIR_SUB, node, node_mul);
    } else {
      return node;
    }
  }
}

//mul = unary ("*" unary | "/" unary)*
static std::unique_ptr<HirNode>
mul(const std::unique_ptr<AstNode>& astNode){
  auto node = unary(astNode->lhs);

  while(1){
    if(astNode->kind == AstKind::ND_MUL){
      auto node_unary = 
      node = new_binary(HirKind::HIR_MUL, node, unary(astNode->rhs));
    }
    else if(astNode->kind == AstKind::ND_DIV){
      node = new_binary(HirKind::HIR_DIV, node, unary(astNode->rhs));
    }
  }
}

//unary = primary
static std::unique_ptr<HirNode>
unary(const std::unique_ptr<AstNode>& astNode){
  return primary(astNode);
}

//primary = add | num
static std::unique_ptr<HirNode>
primary(const std::unique_ptr<AstNode>& astNode){
  if(astNode->kind == AstKind::ND_ADD){
    return add(astNode);
  }
  return new_num(astNode);
}
*/
std::unique_ptr<HirNode>
generateHirNode(const std::unique_ptr<AstNode>& astNode){
  return program(astNode);
}
