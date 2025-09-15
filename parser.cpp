#include "ycc.hpp"

static std::unique_ptr<AstNode> new_node(AstKind kind){
  auto astNode = std::make_unique<AstNode>();
  astNode->kind = kind;
  return astNode;
}

static std::unique_ptr<AstNode> new_binary(AstKind kind,
				    std::unique_ptr<AstNode>& lhs,
				    std::unique_ptr<AstNode>& rhs){
  auto node = new_node(kind);
  node->lhs = std::move(lhs);
  node->rhs = std::move(rhs);
  return node;
}

static std::unique_ptr<AstNode> new_num(int val){
  auto node = new_node(AstKind::AST_NUM);
  node->val = val;
  return node;
}

static std::unique_ptr<AstNode> add();
static std::unique_ptr<AstNode> mul();
static std::unique_ptr<AstNode> unary();
static std::unique_ptr<AstNode> primary();
static std::unique_ptr<AstNode> equality();
static std::unique_ptr<AstNode> relational();

//program = add
std::unique_ptr<AstNode> program(){
  return equality();
}

//equality = relational ("==" relational | "!=" relational)*
static std::unique_ptr<AstNode> equality(){
  auto node = relational();
  while(1){
    if(consume_symbol(TokenType::EQ)){
      auto node_rel = relational();
      node = new_binary(AstKind::AST_EQ, node, node_rel);
    } else if(consume_symbol(TokenType::NE)){
      auto node_rel = relational();
      node = new_binary(AstKind::AST_NE, node, node_rel);
    } else {
      return node;
    }
  }
}

//relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static std::unique_ptr<AstNode> relational(){
  auto node = add();
  while(1){
    if(consume_symbol(TokenType::LT)){
      auto node_add = add();
      node = new_binary(AstKind::AST_LT, node, node_add);
    } else if(consume_symbol(TokenType::LE)){
      auto node_add = add();
      node = new_binary(AstKind::AST_LE, node, node_add);
    } else if(consume_symbol(TokenType::GT)){
      auto node_add = add();
      node = new_binary(AstKind::AST_LT, node_add, node);
    } else if(consume_symbol(TokenType::GE)){
      auto node_add = add();
      node = new_binary(AstKind::AST_LE, node_add, node);
    } else {
      return node;
    }
  }
}

//add = mul ("+" mul | "-" mul)*
static std::unique_ptr<AstNode> add(){
  auto node = mul();

  while(1){
    if(consume_symbol(TokenType::PLUS)){
      auto node_mul = mul();
      node = new_binary(AstKind::AST_ADD, node, node_mul);
    } else if(consume_symbol(TokenType::MINUS)){
      auto node_mul = mul();
      node = new_binary(AstKind::AST_SUB, node, node_mul);
    } else {
      return node;
    }
  }
}

//mul = unary ("*" unary | "/" unary)*
static std::unique_ptr<AstNode> mul(){
  auto node = unary();

  while(1){
    if(consume_symbol(TokenType::STAR)){
      auto node_unary = unary();
      node = new_binary(AstKind::AST_MUL, node, node_unary);
    } else if(consume_symbol(TokenType::SLASH)){
      auto node_unary = unary();
      node = new_binary(AstKind::AST_DIV, node, node_unary);
    } else {
      return node;
    }
  }
}

//unary = primary
static std::unique_ptr<AstNode> unary(){
  return primary();
}

//primary = "(" add ")" | num
static std::unique_ptr<AstNode> primary(){
  if(consume_symbol(TokenType::PAREN_L)){
    auto node = add();
    expect(TokenType::PAREN_R);
    return node;
  }

  return new_num(expect_number());
}
