#include "ycc.hpp"

namespace myParser {
  static std::unordered_map<std::string, std::shared_ptr<Lunaria::Var>> localVars;
  static std::unordered_map<std::string, std::shared_ptr<Lunaria::Var>> globalVars;
  
  static std::shared_ptr<Lunaria::Var> findLvar(const std::unique_ptr<myTokenizer::Token>& token){
    std::shared_ptr<Lunaria::Var> lvar = nullptr;
    if(localVars.contains(token->str)){
      lvar = localVars[token->str];
    }
    return lvar;
  }

  static std::shared_ptr<Lunaria::Var> findGvar(const std::unique_ptr<myTokenizer::Token>& token){
    std::shared_ptr<Lunaria::Var> gvar = nullptr;
    if(globalVars.contains(token->str)){
      gvar = globalVars[token->str];
    }
    return gvar;
  }

  static std::shared_ptr<Lunaria::Var> new_var(const std::string& name, const std::shared_ptr<Lunaria::Type>& type, bool isLocal){
    auto var = std::make_shared<Lunaria::Var>();
    var->name = name;
    var->type = type;
    var->isLocal = isLocal;
    return var;
  }
  
  static std::shared_ptr<Lunaria::Var> new_lvar(const std::string& name, const std::shared_ptr<Lunaria::Type>& type){
    auto lvar = new_var(name, type, true);
    localVars[lvar->name] = lvar;
    return lvar;
  }

  static std::shared_ptr<Lunaria::Var> new_gvar(const std::string& name, const std::shared_ptr<Lunaria::Type>& type, bool isLiteral, const std::string& literal){
    auto gvar = new_var(name, type, false);
    gvar->isLiteral = isLiteral;
    gvar->literal = literal;
    globalVars[gvar->name] = gvar;
    return gvar;
  }

static std::unique_ptr<AstNode> new_node(AstKind kind){
  auto astNode = std::make_unique<AstNode>();
  astNode->kind = kind;
  return astNode;
}

  static std::unique_ptr<AstNode> new_var_node(const std::shared_ptr<Lunaria::Var>& v){
    auto astNode = new_node(AstKind::AST_VAR);
    astNode->var = v;
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
    return look(myTokenizer::TokenType::INT)
      || look(myTokenizer::TokenType::CHAR)
      || look(myTokenizer::TokenType::SHORT)
      || look(myTokenizer::TokenType::LONG)
      ;
  }

  static bool isNotBuildinType(myTokenizer::TokenType t){
    if(t != myTokenizer::TokenType::INT
       && t != myTokenizer::TokenType::CHAR
       && t != myTokenizer::TokenType::SHORT
       && t != myTokenizer::TokenType::LONG){
      return true;
    }
    return false;
  }
  
  static bool isFunction(){
    bool isFunc = false;

    //basetype()
    if(isNotBuildinType(myTokenizer::tokens[0]->tokenType)){
      std::cerr << "parse error in isFunction()" << std::endl;
      exit(1);
    }
    int i = 1;
    while(myTokenizer::tokens[i]->tokenType == myTokenizer::TokenType::STAR){
      i++;
    }

    //expect_ident()
    if(myTokenizer::tokens[i]->tokenType != myTokenizer::TokenType::IDENT){
      std::cerr << "parse error in isFunction()" << std::endl;
      exit(1);
    }
    i++;

    //consume_symbol("(")
    if(myTokenizer::tokens[i]->tokenType == myTokenizer::TokenType::PAREN_L){
      isFunc = true;
    }
    return isFunc;
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
static std::unique_ptr<AstNode> postfix();
static std::unique_ptr<AstNode> primary();
  static std::shared_ptr<Lunaria::Type> basetype();
  static std::shared_ptr<Lunaria::Type> type_suffix(std::shared_ptr<Lunaria::Type>&);

  //global_var = basetype ident type_suffix ";"
  void global_var(){
    auto type = basetype();
    auto name = myTokenizer::expect_ident();
    type = type_suffix(type);
    expect(myTokenizer::TokenType::SEMICOLON);
    auto gvar = new_gvar(name, type, false, "");
    return;
  } //global_var()
  
//program = (global_var | function)*
std::unique_ptr<Program> program(){
  globalVars.clear();
  std::list<std::unique_ptr<Function>> fns;
  while(!myTokenizer::at_eof()){
    if(isFunction()){
      auto fn = function();
      if(!fn){
	continue;
      }
      fns.push_back(std::move(fn));
      continue;
    } else {
      global_var();
    }
  } //while
  auto prog = std::make_unique<Program>();
  prog->fns = std::move(fns);
  prog->globalVars = globalVars;
  return prog;
}
  
  //basetype = builtin-type "*"*
  //builtin-type = "int" | "char" | "short" | "long"
  static std::shared_ptr<Lunaria::Type> basetype(){
    //expect(myTokenizer::TokenType::INT);
    //auto type = Lunaria::int_type;
    std::shared_ptr<Lunaria::Type> type = nullptr;
    if(myTokenizer::consume_symbol(myTokenizer::TokenType::INT)){
      type = Lunaria::int_type;
    } else if(myTokenizer::consume_symbol(myTokenizer::TokenType::CHAR)){
      type = Lunaria::char_type;
    } else if(myTokenizer::consume_symbol(myTokenizer::TokenType::SHORT)){
      type = Lunaria::short_type;
    } else if(myTokenizer::consume_symbol(myTokenizer::TokenType::LONG)){
      type = Lunaria::long_type;
    }
    
    while(myTokenizer::consume_symbol(myTokenizer::TokenType::STAR)){
      type = Lunaria::pointer_to(type);
    }
    return type;
  }

//param = basetype ident
static std::shared_ptr<Lunaria::Var> readFuncParam(){
  const auto type = basetype();
  const auto token = myTokenizer::consume_ident();
  if(token){
    return new_lvar(token->str, type);
  }
  return nullptr;
}
  
//params = param ("," param)*
static std::list<std::shared_ptr<Lunaria::Var>> readFuncParams(){
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::PAREN_R)){
    //no params
    return std::list<std::shared_ptr<Lunaria::Var>>();
  }
  std::list<std::shared_ptr<Lunaria::Var>> params;
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
      return new_binary(AstKind::AST_PTR_DIFF, lhs, rhs);
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
//        | ("++" | "--") unary
//        | "sizeof" unary
//        | postfix
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
  
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::PLUSPLUS)){
    auto node = new_node(AstKind::AST_PRE_INC);
    node->lhs = unary();
    return node;
  }
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::MINUSMINUS)){
    auto node = new_node(AstKind::AST_PRE_DEC);
    node->lhs = unary();
    return node;
  }
  
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::SIZEOF)){
    auto node_unary = unary();
    add_type(node_unary);
    return new_num(node_unary->type->size);
  }
  return postfix();
}

  //postfix = primary ("[" expr "]")*
  static std::unique_ptr<AstNode> postfix(){
    auto node = primary();
    while(true){
      if(consume_symbol(myTokenizer::TokenType::BRACKET_L)){
	//a[b] => AST_SUBSCRIPTED, lhs=a, rhs=b
	auto node_expr = expr();
	//auto tmp = new_add(node, expr());
	expect(myTokenizer::TokenType::BRACKET_R);
	//node = new_unary(ND_DEREF, tmp);
	node = new_binary(AstKind::AST_SUBSCRIPTED, node, node_expr);
	continue;
      } 
      break;
    } 
    return node;
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

  static std::string new_label(){
    static int count = 0;
    const std::string label = ".L.data." + std::to_string(count);
    ++count;
    return label;
  }

//primary = "(" expr ")"
//          | ident funcArgs?
//          | string_literal
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
    auto node = new_node(AstKind::AST_VAR);
    auto lvar = findLvar(token);
    auto gvar = findGvar(token);
    if(lvar){
      node->var = lvar;
    } else if(gvar){
      node->var = gvar;
    } else {      
      std::cerr << "undefined variable" << std::endl;
      exit(1);
    }
    return node;
  } //if(token)

  auto tok_str = myTokenizer::consume_str();
  if(tok_str){
    //string_literal
    auto type = Lunaria::array_of(Lunaria::char_type, tok_str->literal.size());
    auto gvar = new_gvar(new_label(), type, true, tok_str->literal);
    return new_var_node(gvar);
  }
  
  return new_num(myTokenizer::expect_number());
}
} //namespace myParser
