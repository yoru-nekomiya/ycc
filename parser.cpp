#include "ycc.hpp"

namespace myParser {
  static std::unordered_map<std::string, std::shared_ptr<Lunaria::LVar>> localVars;
  std::shared_ptr<Lunaria::LVar> findLvar(const std::unique_ptr<myTokenizer::Token>& token){
  std::shared_ptr<Lunaria::LVar> lvar = nullptr;
  if(localVars.contains(token->str)){
    lvar = localVars[token->str];
  }
  return lvar;
}
  std::shared_ptr<Lunaria::LVar> new_lvar(const std::string& name, const std::shared_ptr<Lunaria::Type>& type){
    auto lvar = std::make_shared<Lunaria::LVar>();
    lvar->name = name;
    lvar->type = type;
    localVars[lvar->name] = lvar;
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

  static bool isTypeName(){
    return look(myTokenizer::TokenType::INT);
  }

  static std::unique_ptr<Function> function();
static std::unique_ptr<AstNode> stmt();
static std::unique_ptr<AstNode> stmt2();
static std::unique_ptr<AstNode> expr();
static std::unique_ptr<AstNode> assign();
static std::unique_ptr<AstNode> equality();
static std::unique_ptr<AstNode> relational();
static std::unique_ptr<AstNode> add();
static std::unique_ptr<AstNode> mul();
static std::unique_ptr<AstNode> unary();
static std::unique_ptr<AstNode> primary();

//program = function*
std::unique_ptr<Program> program(){
  std::list<std::unique_ptr<Function>> fns;
  while(!myTokenizer::at_eof()){
    auto fn = function();
    if(!fn){
      continue;
    }
    fns.push_back(std::move(fn));
    continue;
  }
  auto prog = std::make_unique<Program>();
  prog->fns = std::move(fns);
  return prog;
}
  
  //basetype = "int" "*"*
  static std::shared_ptr<Lunaria::Type> basetype(){
    expect(myTokenizer::TokenType::INT);
    auto type = Lunaria::int_type;
    while(myTokenizer::consume_symbol(myTokenizer::TokenType::STAR)){
      type = Lunaria::pointer_to(type);
    }
    return type;
  }

//param = basetype ident
static std::shared_ptr<Lunaria::LVar> readFuncParam(){
  const auto type = basetype();
  const auto token = myTokenizer::consume_ident();
  if(token){
    return new_lvar(token->str, type);
  }
  return nullptr;
}
  
//params = param ("," param)*
static std::list<std::shared_ptr<Lunaria::LVar>> readFuncParams(){
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::PAREN_R)){
    //no params
    return std::list<std::shared_ptr<Lunaria::LVar>>();
  }
  std::list<std::shared_ptr<Lunaria::LVar>> params;
  params.push_back(readFuncParam());
  while(!myTokenizer::consume_symbol(myTokenizer::TokenType::PAREN_R)){
    myTokenizer::expect(myTokenizer::TokenType::COMMA);
    params.push_back(readFuncParam());
  }
  return params;
}

//function = basetype ident "(" params? ")" "{" stmt* "}"
//params = param ("," param)*
//param = basetype ident
static std::unique_ptr<Function> function(){
  localVars.clear();

  const auto type = basetype();
  const auto funcName = myTokenizer::expect_ident();
  auto fn = std::make_unique<Function>();
  fn->name = funcName;
  myTokenizer::expect(myTokenizer::TokenType::PAREN_L);
  fn->params = readFuncParams();

  myTokenizer::expect(myTokenizer::TokenType::BRACE_L);
  while(!myTokenizer::consume_symbol(myTokenizer::TokenType::BRACE_R)){
    fn->body.push_back(stmt());
  }
  fn->localVars = localVars;
  return fn;
}

  static std::shared_ptr<Lunaria::Type> type_suffix(std::shared_ptr<Lunaria::Type>&);
  static int const_expr();
  static int eval(const std::unique_ptr<AstNode>&);

//declaration = basetype ident type_suffix ";"
  static std::unique_ptr<AstNode> declaration(){
    auto type = basetype();
    const auto name = myTokenizer::expect_ident();
    type = type_suffix(type);
    myTokenizer::expect(myTokenizer::TokenType::SEMICOLON);

    auto lvar = new_lvar(name, type);
    return new_node(AstKind::AST_NULL);
  }

  //type_suffix = ("[" const_expr "]" type_suffix)?
  static std::shared_ptr<Lunaria::Type> type_suffix(std::shared_ptr<Lunaria::Type>& type){
    if(!myTokenizer::consume_symbol(myTokenizer::TokenType::BRACKET_L)){
      return type;
    }
    
    int size = 0;
    if(!myTokenizer::consume_symbol(myTokenizer::TokenType::BRACKET_R)){
      size = const_expr();
      myTokenizer::expect(myTokenizer::TokenType::BRACKET_R);
    } //if
    
    type = type_suffix(type);
    type = Lunaria::array_of(type, size);
    return type;
  }

  static int const_expr(){
    return eval(add());
  }

  static int eval(const std::unique_ptr<AstNode>& node){
    switch(node->kind){
    case AstKind::AST_ADD:
      return eval(node->lhs) + eval(node->rhs);
    case AstKind::AST_SUB:
      return eval(node->lhs) - eval(node->rhs);
    case AstKind::AST_MUL:
      return eval(node->lhs) * eval(node->rhs);
    case AstKind::AST_DIV:
      return eval(node->lhs) / eval(node->rhs);
    case AstKind::AST_NUM:
      return node->val;   
    } 
    std::cerr << "not a constant expression" << std::endl;
    exit(1);
  }
  
  //stmt = stmt2
  static std::unique_ptr<AstNode> stmt(){
    auto node = stmt2();
    add_type(node);
    return node;
  }
  
//stmt2 = expr ";"
//       | "{" stmt* "}"
//       | "return" expr ";"
//       | "if" "(" expr ")" stmt ("else" stmt)?
//       | "while" "(" expr ")" stmt
//       | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//       | declaration
static std::unique_ptr<AstNode> stmt2(){
  std::unique_ptr<AstNode> node;
  
  //"return" expr ";"
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::RETURN)){
    node = new_node(AstKind::AST_RETURN);
    auto node_expr = expr();
    node->lhs = std::move(node_expr);
    myTokenizer::expect(myTokenizer::TokenType::SEMICOLON);
    return node;
  }

  //"if" "(" expr ")" stmt ("else" stmt)?
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::IF)){
    node = new_node(AstKind::AST_IF);
    myTokenizer::expect(myTokenizer::TokenType::PAREN_L);
    node->cond = expr();
    myTokenizer::expect(myTokenizer::TokenType::PAREN_R);
    node->then = stmt();
    if(myTokenizer::consume_symbol(myTokenizer::TokenType::ELSE)){
      node->els = stmt();
    }
    return node;
  }

  //"while" "(" expr ")" stmt
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::WHILE)){
    node = new_node(AstKind::AST_WHILE);
    myTokenizer::expect(myTokenizer::TokenType::PAREN_L);
    node->cond = expr();
    myTokenizer::expect(myTokenizer::TokenType::PAREN_R);
    node->then = stmt();
    return node;
  }

  //"for" "(" expr? ";" expr? ";" expr? ")" stmt
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::FOR)){
    node = new_node(AstKind::AST_FOR);
    myTokenizer::expect(myTokenizer::TokenType::PAREN_L);    
    if(!myTokenizer::consume_symbol(myTokenizer::TokenType::SEMICOLON)){
      //first expr
      node->init = expr();
      myTokenizer::expect(myTokenizer::TokenType::SEMICOLON);
    }
    if(!myTokenizer::consume_symbol(myTokenizer::TokenType::SEMICOLON)){
      //second expr
      node->cond = expr();
      myTokenizer::expect(myTokenizer::TokenType::SEMICOLON);
    }
    if(!myTokenizer::consume_symbol(myTokenizer::TokenType::PAREN_R)){
      //third expr
      node->inc = expr();
      myTokenizer::expect(myTokenizer::TokenType::PAREN_R);
    }
    node->then = stmt();
    return node;
  }

  //"{" stmt* "}"
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::BRACE_L)){
    node = new_node(AstKind::AST_BLOCK);
    while(!myTokenizer::consume_symbol(myTokenizer::TokenType::BRACE_R)){
      node->body.push_back(stmt());
    }
    return node;
  }

  //declaration
  if(isTypeName()){
    return declaration();
  }
  
  //expr ";"
  node = expr();
  myTokenizer::expect(myTokenizer::TokenType::SEMICOLON);
  return node;
}

// expr = assign
static std::unique_ptr<AstNode> expr(){
  return assign();
}

//assign = equality ("=" assign)?
static std::unique_ptr<AstNode> assign(){
  auto node = equality();
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::ASSIGN)){
    auto node_assign = assign();
    node = new_binary(AstKind::AST_ASSIGN, node, node_assign);
  }
  return node;
}

//equality = relational ("==" relational | "!=" relational)*
static std::unique_ptr<AstNode> equality(){
  auto node = relational();
  while(1){
    if(myTokenizer::consume_symbol(myTokenizer::TokenType::EQ)){
      auto node_rel = relational();
      node = new_binary(AstKind::AST_EQ, node, node_rel);
    } else if(myTokenizer::consume_symbol(myTokenizer::TokenType::NE)){
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
    if(myTokenizer::consume_symbol(myTokenizer::TokenType::LT)){
      auto node_add = add();
      node = new_binary(AstKind::AST_LT, node, node_add);
    } else if(myTokenizer::consume_symbol(myTokenizer::TokenType::LE)){
      auto node_add = add();
      node = new_binary(AstKind::AST_LE, node, node_add);
    } else if(myTokenizer::consume_symbol(myTokenizer::TokenType::GT)){
      auto node_add = add();
      node = new_binary(AstKind::AST_LT, node_add, node);
    } else if(myTokenizer::consume_symbol(myTokenizer::TokenType::GE)){
      auto node_add = add();
      node = new_binary(AstKind::AST_LE, node_add, node);
    } else {
      return node;
    }
  }
}

  static std::unique_ptr<AstNode> new_add(std::unique_ptr<AstNode>& lhs, std::unique_ptr<AstNode>& rhs){
    add_type(lhs);
    add_type(rhs);
    
    if(is_integer(lhs->type) && is_integer(rhs->type)){
      //lhs:int rhs:int
      return new_binary(AstKind::AST_ADD, lhs, rhs);
    }
    if(lhs->type->base && is_integer(rhs->type)){
      //lhs:pointer rhs:int
      return new_binary(AstKind::AST_PTR_ADD, lhs, rhs);
    }
    if(is_integer(lhs->type) && rhs->type->base){
      //lhs:int rhs:pointer
      return new_binary(AstKind::AST_PTR_ADD, rhs, lhs);
    }
    std::cerr << "invalid operands" << std::endl;
    exit(1);
  }

  static std::unique_ptr<AstNode> new_sub(std::unique_ptr<AstNode>& lhs, std::unique_ptr<AstNode>& rhs){
    add_type(lhs);
    add_type(rhs);
    
    if(is_integer(lhs->type) && is_integer(rhs->type)){
      //lhs:int rhs:int
      return new_binary(AstKind::AST_SUB, lhs, rhs);
    }
    if(lhs->type->base && is_integer(rhs->type)){
      //lhs:pointer rhs:int
      return new_binary(AstKind::AST_PTR_SUB, lhs, rhs);
    }
    if(lhs->type->base && rhs->type->base){
      //lhs:pointer rhs:pointer
      return new_binary(AstKind::AST_PTR_DIFF, rhs, lhs);
    }
    std::cerr << "invalid operands" << std::endl;
    exit(1);
  }
  
//add = mul ("+" mul | "-" mul)*
static std::unique_ptr<AstNode> add(){
  auto node = mul();

  while(1){
    if(myTokenizer::consume_symbol(myTokenizer::TokenType::PLUS)){
      auto node_mul = mul();
      //node = new_binary(AstKind::AST_ADD, node, node_mul);
      node = new_add(node, node_mul);
    } else if(myTokenizer::consume_symbol(myTokenizer::TokenType::MINUS)){
      auto node_mul = mul();
      //node = new_binary(AstKind::AST_SUB, node, node_mul);
      node = new_sub(node, node_mul);
    } else {
      return node;
    }
  }
}

//mul = unary ("*" unary | "/" unary)*
static std::unique_ptr<AstNode> mul(){
  auto node = unary();

  while(1){
    if(myTokenizer::consume_symbol(myTokenizer::TokenType::STAR)){
      auto node_unary = unary();
      node = new_binary(AstKind::AST_MUL, node, node_unary);
    } else if(myTokenizer::consume_symbol(myTokenizer::TokenType::SLASH)){
      auto node_unary = unary();
      node = new_binary(AstKind::AST_DIV, node, node_unary);
    } else {
      return node;
    }
  }
}

//unary = ("+" | "-" | "*" | "&")? unary
//        | "sizeof" unary
//        | primary
static std::unique_ptr<AstNode> unary(){
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::PLUS)){
    return unary();
  }
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::MINUS)){
    auto node_unary = unary();
    auto node_zero = new_num(0);
    return new_binary(AstKind::AST_SUB, node_zero, node_unary);
  }
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::STAR)){
    auto node_deref = new_node(AstKind::AST_DEREF);
    node_deref->lhs = unary();
    return node_deref;
  }
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::AND)){
    auto node_addr = new_node(AstKind::AST_ADDR);
    node_addr->lhs = unary();
    return node_addr;
  }
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::SIZEOF)){
    auto node_unary = unary();
    add_type(node_unary);
    return new_num(node_unary->type->size);
  }
  return primary();
}

static std::list<std::unique_ptr<AstNode>> funcArgs(){
  std::list<std::unique_ptr<AstNode>> lirList = {};
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::PAREN_R)){
    //no function args
    return lirList;
  }
  auto node = assign();
  lirList.push_back(std::move(node));
  while(myTokenizer::consume_symbol(myTokenizer::TokenType::COMMA)){
    node = assign();
    lirList.push_back(std::move(node));
  }
  myTokenizer::expect(myTokenizer::TokenType::PAREN_R);
  return lirList;
}

//primary = "(" expr ")"
//          | ident funcArgs?
//          | num
static std::unique_ptr<AstNode> primary(){
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::PAREN_L)){
    auto node_expr = expr();
    myTokenizer::expect(myTokenizer::TokenType::PAREN_R);
    return node_expr;
  }

  auto token = myTokenizer::consume_ident();
  if(token){
    if(myTokenizer::consume_symbol(myTokenizer::TokenType::PAREN_L)){
      //function call
      auto node = new_node(AstKind::AST_FUNCALL);
      node->funcName = token->str;
      node->args = funcArgs();
      return node;
    }

    //variable
    auto node = new_node(AstKind::AST_LVAR);
    auto lvar = findLvar(token);
    if(lvar){
      node->lvar = lvar;
    } else {      
      std::cerr << "undefined variable" << std::endl;
      exit(1);
    }
    return node;
  } //if(token)

  return new_num(myTokenizer::expect_number());
}
} //namespace myParser
