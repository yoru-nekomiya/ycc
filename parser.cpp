#include "ycc.hpp"

std::unordered_map<std::string, std::shared_ptr<LVar>> localVars;
std::shared_ptr<LVar> findLvar(const std::unique_ptr<Token>& token){
  std::shared_ptr<LVar> lvar = nullptr;
  if(localVars.contains(token->str)){
    lvar = localVars[token->str];
  }
  return lvar;
}

static std::unique_ptr<AstNode> new_node(AstKind kind){
  auto astNode = std::make_unique<AstNode>();
  astNode->kind = kind;
  return astNode;
}

static std::unique_ptr<AstNode>
new_binary(AstKind kind,
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

static std::unique_ptr<AstNode> stmt();
static std::unique_ptr<AstNode> expr();
static std::unique_ptr<AstNode> assign();
static std::unique_ptr<AstNode> equality();
static std::unique_ptr<AstNode> relational();
static std::unique_ptr<AstNode> add();
static std::unique_ptr<AstNode> mul();
static std::unique_ptr<AstNode> unary();
static std::unique_ptr<AstNode> primary();

//program = stmt*
std::list<std::unique_ptr<AstNode>> program(){
  std::list<std::unique_ptr<AstNode>> astNodeList;
  while(!at_eof()){
    astNodeList.push_back(stmt());
  }
  return astNodeList;
}

//stmt = expr ";"
//       | "{" stmt* "}"
//       | "return" expr ";"
//       | "if" "(" expr ")" stmt ("else" stmt)?
//       | "while" "(" expr ")" stmt
//       | "for" "(" expr? ";" expr? ";" expr? ")" stmt
static std::unique_ptr<AstNode> stmt(){
  std::unique_ptr<AstNode> node;
  
  //"return" expr ";"
  if(consume_symbol(TokenType::RETURN)){
    node = new_node(AstKind::AST_RETURN);
    auto node_expr = expr();
    node->lhs = std::move(node_expr);
    expect(TokenType::SEMICOLON);
    return node;
  }

  //"if" "(" expr ")" stmt ("else" stmt)?
  if(consume_symbol(TokenType::IF)){
    node = new_node(AstKind::AST_IF);
    expect(TokenType::PAREN_L);
    node->cond = expr();
    expect(TokenType::PAREN_R);
    node->then = stmt();
    if(consume_symbol(TokenType::ELSE)){
      node->els = stmt();
    }
    return node;
  }

  //"while" "(" expr ")" stmt
  if(consume_symbol(TokenType::WHILE)){
    node = new_node(AstKind::AST_WHILE);
    expect(TokenType::PAREN_L);
    node->cond = expr();
    expect(TokenType::PAREN_R);
    node->then = stmt();
    return node;
  }

  //"for" "(" expr? ";" expr? ";" expr? ")" stmt
  if(consume_symbol(TokenType::FOR)){
    node = new_node(AstKind::AST_FOR);
    expect(TokenType::PAREN_L);    
    if(!consume_symbol(TokenType::SEMICOLON)){
      //first expr
      node->init = expr();
      expect(TokenType::SEMICOLON);
    }
    if(!consume_symbol(TokenType::SEMICOLON)){
      //second expr
      node->cond = expr();
      expect(TokenType::SEMICOLON);
    }
    if(!consume_symbol(TokenType::PAREN_R)){
      //third expr
      node->inc = expr();
      expect(TokenType::PAREN_R);
    }
    node->then = stmt();
    return node;
  }

  //"{" stmt* "}"
  if(consume_symbol(TokenType::BRACE_L)){
    node = new_node(AstKind::AST_BLOCK);
    while(!consume_symbol(TokenType::BRACE_R)){
      node->body.push_back(stmt());
    }
    return node;
  }
  
  //expr ";"
  node = expr();
  expect(TokenType::SEMICOLON);
  return node;
}

// expr = assign
static std::unique_ptr<AstNode> expr(){
  return assign();
}

//assign = equality ("=" assign)?
static std::unique_ptr<AstNode> assign(){
  auto node = equality();
  if(consume_symbol(TokenType::ASSIGN)){
    auto node_assign = assign();
    node = new_binary(AstKind::AST_ASSIGN, node, node_assign);
  }
  return node;
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

//unary = ("+" | "-")? unary
//        | primary
static std::unique_ptr<AstNode> unary(){
  if(consume_symbol(TokenType::PLUS)){
    return unary();
  }
  if(consume_symbol(TokenType::MINUS)){
    auto node_unary = unary();
    auto node_zero = new_num(0);
    return new_binary(AstKind::AST_SUB, node_zero, node_unary);
  }
  return primary();
}

//primary = "(" expr ")" | ident | num
static std::unique_ptr<AstNode> primary(){
  if(consume_symbol(TokenType::PAREN_L)){
    auto node_expr = expr();
    expect(TokenType::PAREN_R);
    return node_expr;
  }

  auto token = consume_ident();
  if(token){
    auto node = new_node(AstKind::AST_LVAR);
    auto lvar = findLvar(token);
    if(lvar){
      node->lvar = lvar;
    } else {    
      auto lvar = std::make_shared<LVar>();
      lvar->name = token->str;  
      lvar->offset = (localVars.size()+1) * 8;
      node->lvar = lvar;
      localVars[lvar->name] = lvar;
    }
    return node;
  } //if(token)

  return new_num(expect_number());
}
