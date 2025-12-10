#include "ycc.hpp"

namespace myParser {
  static std::unordered_set<std::shared_ptr<Lunaria::Var>,Lunaria::VarSharedPtrHash,Lunaria::VarSharedPtrEqual> localVars;
  static std::unordered_set<std::shared_ptr<Lunaria::Var>,Lunaria::VarSharedPtrHash,Lunaria::VarSharedPtrEqual> globalVars;
  static std::stack<int> breaks = {};
  static std::stack<int> continues = {};
  static std::stack<int> switches = {};

  struct VarScope{
    std::string name;
    int depth;
    
    std::shared_ptr<Lunaria::Var> var;
  };

  struct Scope{
    std::unordered_map<std::string, std::shared_ptr<VarScope>> vars;
  };

  static std::vector<std::shared_ptr<Scope>> scope = {};
  static int scope_depth = 0;

  static std::shared_ptr<Scope> enter_scope(){
    auto sc = std::make_shared<Scope>();
    scope_depth++;
    scope.push_back(sc);
    return sc;
  }

  static void leave_scope(){
    scope_depth--;
    scope.pop_back();
    return;
  }

  static std::shared_ptr<VarScope> find_var_scope(const std::unique_ptr<myTokenizer::Token>& token){
    for(auto iter = scope.rbegin(); iter != scope.rend(); iter++){
      auto vars = (*iter)->vars;
      auto it_v = vars.find(token->str);
      if(it_v != vars.end()){
	return it_v->second;
      }
    }
    return nullptr;
  }

  static std::shared_ptr<VarScope> find_var_current_scope(const std::string& name){
    auto it_v = scope.back()->vars.find(name);
    if(it_v != scope.back()->vars.end()){
      return it_v->second;
    }
    return nullptr;
  }

  static std::shared_ptr<VarScope> push_to_current_scope(const std::string& name){
    auto vsc = std::make_shared<VarScope>();
    vsc->name = name;
    vsc->depth = scope_depth;
    const auto pair = std::make_pair(name, vsc);
    if(find_var_current_scope(name) != nullptr){
      //redeclaration
      std::cerr << "redeclaration of variable\n";
    }
    scope.back()->vars.insert(pair);
    return vsc;
  }
  
  static std::shared_ptr<Lunaria::Var> new_var(const std::string& name, const std::shared_ptr<Lunaria::Type>& type, bool isLocal){
    auto var = std::make_shared<Lunaria::Var>(name, type, isLocal);    
    return var;
  }

  static int id_local_var = 0;
  static std::shared_ptr<Lunaria::Var> new_lvar(const std::string& name, const std::shared_ptr<Lunaria::Type>& type){
    auto lvar = new_var(name, type, true);
    lvar->id = id_local_var++;
    localVars.insert(lvar);
    auto vsc = push_to_current_scope(name);
    vsc->var = lvar;
    return lvar;
  }

  static std::shared_ptr<Lunaria::Var> new_gvar(const std::string& name, const std::shared_ptr<Lunaria::Type>& type, bool isLiteral, const std::string& literal){
    static int id = 0;
    auto gvar = new_var(name, type, false);
    gvar->id = id++;
    gvar->isLiteral = isLiteral;
    gvar->literal = literal;
    globalVars.insert(gvar);
    auto vsc = push_to_current_scope(name);
    vsc->var = gvar;
    return gvar;
  }

static std::unique_ptr<AstNode> new_node(AstKind kind){
  static int id = 0;
  auto astNode = std::make_unique<AstNode>();
  astNode->id = id++;
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

static std::unique_ptr<AstNode> new_num(long long val){
  auto node = new_node(AstKind::AST_NUM);
  node->val = val;
  return node;
}

  static bool isTypeName(){
    return look(myTokenizer::TokenType::INT)
      || look(myTokenizer::TokenType::CHAR)
      || look(myTokenizer::TokenType::SHORT)
      || look(myTokenizer::TokenType::LONG)
      || look(myTokenizer::TokenType::VOID)
      || look(myTokenizer::TokenType::STRUCT)
      ;
  }

  static bool isNotBuiltinType(myTokenizer::TokenType t){
    if(t != myTokenizer::TokenType::INT
       && t != myTokenizer::TokenType::CHAR
       && t != myTokenizer::TokenType::SHORT
       && t != myTokenizer::TokenType::LONG
       && t != myTokenizer::TokenType::VOID
       && t != myTokenizer::TokenType::STRUCT
       ){
      return true;
    }
    return false;
  }
  
  static bool isFunction(){
    bool isFunc = false;

    //basetype()
    if(isNotBuiltinType(myTokenizer::tokens[0]->tokenType)){
      std::cerr << "parse error in isFunction()" << std::endl;
      exit(1);
    }
    int i = 1;
    while(myTokenizer::tokens[i]->tokenType == myTokenizer::TokenType::STAR){
      i++;
    }

    if(myTokenizer::tokens[i]->tokenType != myTokenizer::TokenType::SEMICOLON){
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
    } //if token != ";"
    return isFunc;
  }

  static std::unique_ptr<Function> function();
  static std::unique_ptr<AstNode> stmt();
  static std::unique_ptr<AstNode> stmt2();
  static std::unique_ptr<AstNode> expr();
  static std::unique_ptr<AstNode> assign();
  static std::unique_ptr<AstNode> conditional();
  static std::unique_ptr<AstNode> logor();
  static std::unique_ptr<AstNode> logand();
  static std::unique_ptr<AstNode> _bit_or();
  static std::unique_ptr<AstNode> _bit_xor();
  static std::unique_ptr<AstNode> _bit_and();
  static std::unique_ptr<AstNode> equality();
  static std::unique_ptr<AstNode> relational();
  static std::unique_ptr<AstNode> shift();
  static std::unique_ptr<AstNode> add();
  static std::unique_ptr<AstNode> mul();
  static std::unique_ptr<AstNode> unary();
  static std::unique_ptr<AstNode> postfix();
  static std::unique_ptr<AstNode> primary();
  static std::shared_ptr<Lunaria::Type> basetype();
  static std::shared_ptr<Lunaria::Type> type_suffix(std::shared_ptr<Lunaria::Type>&);
  static std::shared_ptr<Lunaria::Type> struct_decl();
  static long const_expr();
  static long eval(const std::unique_ptr<AstNode>&);
  static long eval2(const std::unique_ptr<AstNode>& node, std::shared_ptr<std::shared_ptr<Lunaria::Var>>& v);
  static std::unique_ptr<AstNode> new_add(std::unique_ptr<AstNode>& lhs, std::unique_ptr<AstNode>& rhs);
  static std::unique_ptr<AstNode> new_unary(AstKind k, std::unique_ptr<AstNode>& lhs);
  
  static void
  new_init_val(std::vector<std::unique_ptr<Lunaria::Initializer>>& cur,
	       int sz, int val){
    cur.push_back(std::make_unique<Lunaria::Initializer>(sz, val));
  }

  static void
  new_init_zero(std::vector<std::unique_ptr<Lunaria::Initializer>>& cur,
		int nbytes){
    //assign zero, nbytes
    for(int i = 0; i < nbytes; i++){
      new_init_val(cur, 1, 0);
    }
  }

  static void
  new_init_label(std::vector<std::unique_ptr<Lunaria::Initializer>>& cur,
		 const std::string& label,
		 long addend){
    cur.push_back(std::make_unique<Lunaria::Initializer>(label, addend));
  }

  static void skip_excess_elements2(){
    while(1){
      if(myTokenizer::consume_symbol(myTokenizer::TokenType::BRACE_L)){
	skip_excess_elements2();
      } else {
	assign();
      }

      if(myTokenizer::consume_symbol(myTokenizer::TokenType::BRACE_R)){
	return;
      }
      myTokenizer::expect(myTokenizer::TokenType::COMMA);
    } //while
  }
  
  static void skip_excess_elements(){
    myTokenizer::expect(myTokenizer::TokenType::COMMA);
    skip_excess_elements2();
  }

  //gvar_initializer2 = assign
  //                    | "{" (gvar_initializer2 ("," gvar_initializer2)* )? "}" 
  static void
  gvar_initializer2(std::vector<std::unique_ptr<Lunaria::Initializer>>& cur, std::shared_ptr<Lunaria::Type>& type){
    auto& tok = myTokenizer::tokens.front();

    if(type->kind == Lunaria::TypeKind::ARRAY
       && type->base->kind == Lunaria::TypeKind::CHAR
       && tok->tokenType == myTokenizer::TokenType::STR){
      //char a[]="hoge"

      if(type->is_incomplete){
	//when the number of elements is omitted
	type->size = tok->literal.size();
	type->array_size = tok->literal.size();
	type->is_incomplete = false;
      }

      //compare array_size with string_length
      const int len = (type->array_size < tok->literal.size())
	? type->array_size : tok->literal.size();
      for(int i = 0; i < len; i++){
	new_init_val(cur, 1, tok->literal[i]);
      }
      //when type->array_size >= tok->str_len, fill the difference with 0
      new_init_zero(cur, type->array_size - len);
      myTokenizer::tokens.pop_front();
      return;
    } //if ARRAY && CHAR && STR

    if(type->kind == Lunaria::TypeKind::ARRAY){
      //T a[] = {1, 2}
      const bool open = myTokenizer::consume_symbol(myTokenizer::TokenType::BRACE_L);
      //if the number of elements is omitted, any number of elements is allowed.
      const int limit = type->is_incomplete ? INT_MAX : type->array_size;
      int i = 0;
      if(!myTokenizer::look(myTokenizer::TokenType::BRACE_R)){
	do{
	  gvar_initializer2(cur, type->base);
	  i++;
	}while(i < limit && !myTokenizer::look(myTokenizer::TokenType::BRACE_R) && myTokenizer::consume_symbol(myTokenizer::TokenType::COMMA));
      }

      if(open && !myTokenizer::consume_symbol(myTokenizer::TokenType::BRACE_R)){
	//The number of elements is exceeded
	//T a[1] = {1,2}
	skip_excess_elements();
      }

      //set array elements which is not initialized to zero
      //T a[3] = {1}
      new_init_zero(cur, type->base->size * (type->array_size-i));

      if(type->is_incomplete){
	//the number of elements is omitted
	type->size = type->base->size * i;
	type->array_size = i;
	type->is_incomplete = false;
      }
      return;
    } //if ARRAY

    //if(type->kind == STRUCT)
    //TODO:

    const bool open = myTokenizer::consume_symbol(myTokenizer::TokenType::BRACE_L);
    auto expression = conditional();
    if(open){
      myTokenizer::expect(myTokenizer::TokenType::BRACE_R);
    }

    std::shared_ptr<Lunaria::Var> v = nullptr;
    std::shared_ptr<std::shared_ptr<Lunaria::Var>> vv = std::make_shared<std::shared_ptr<Lunaria::Var>>(v);
    const long constant = eval2(expression, vv);
    if(*vv){
      const int scale = ((*vv)->type->kind == Lunaria::TypeKind::ARRAY)
	? (*vv)->type->base->size : (*vv)->type->size;
      new_init_label(cur, (*vv)->name, constant*scale);
      return;
    }
    new_init_val(cur, type->size, constant);
    return;
  }
  
  std::vector<std::unique_ptr<Lunaria::Initializer>>
  gvar_initializer(std::shared_ptr<Lunaria::Type>& type){
    std::vector<std::unique_ptr<Lunaria::Initializer>> vec;
    gvar_initializer2(vec, type);
    return vec;
  }
  
  //global_var = basetype ident type_suffix ("=" gvar_initializer)? ";"
  void global_var(){
    auto type = basetype();
    if(myTokenizer::consume_symbol(myTokenizer::TokenType::SEMICOLON)){
      return;
    }
    
    auto name = myTokenizer::expect_ident();
    type = type_suffix(type);

    if(type->kind == Lunaria::TypeKind::VOID){
      std::cerr << "variable is declared void\n";
      exit(1);
    }
    
    auto gvar = new_gvar(name, type, false, "");

    if(myTokenizer::consume_symbol(myTokenizer::TokenType::ASSIGN)){
      gvar->initializer = gvar_initializer(type);
      expect(myTokenizer::TokenType::SEMICOLON);
      return;
    }

    if(type->is_incomplete){
      std::cerr << "gvar type is incomplete\n";
      exit(1);
    }
    expect(myTokenizer::TokenType::SEMICOLON);
    return;
  } //global_var()
  
//program = (global_var | function)*
std::unique_ptr<Program> program(){
  globalVars.clear();
  scope.push_back(std::make_shared<Scope>());
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
  scope.pop_back();
  return prog;
}
  
  //basetype = (builtin-type | struct-decl) "*"*
  //builtin-type = "int" | "char" | "short" | "long" | "void"
  static std::shared_ptr<Lunaria::Type> basetype(){
    std::shared_ptr<Lunaria::Type> type = nullptr;
    if(myTokenizer::consume_symbol(myTokenizer::TokenType::INT)){
      type = Lunaria::int_type;
    } else if(myTokenizer::consume_symbol(myTokenizer::TokenType::CHAR)){
      type = Lunaria::char_type;
    } else if(myTokenizer::consume_symbol(myTokenizer::TokenType::SHORT)){
      type = Lunaria::short_type;
    } else if(myTokenizer::consume_symbol(myTokenizer::TokenType::LONG)){
      type = Lunaria::long_type;
    } else if(myTokenizer::consume_symbol(myTokenizer::TokenType::VOID)){
      type = Lunaria::void_type;
    }

    if(!look(myTokenizer::TokenType::INT)
       && !look(myTokenizer::TokenType::CHAR)
       && !look(myTokenizer::TokenType::SHORT)
       && !look(myTokenizer::TokenType::LONG)
       && !look(myTokenizer::TokenType::VOID)){
      if(look(myTokenizer::TokenType::STRUCT)){
	type = struct_decl();
      }
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
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::VOID)
     && myTokenizer::consume_symbol(myTokenizer::TokenType::PAREN_R)){
    //param is void
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

//function = basetype ident "(" params? ")" ("{" stmt* "}" | ";")
//params = param ("," param)*
//param = basetype ident
static std::unique_ptr<Function> function(){
  localVars.clear();
  id_local_var = 0;

  const auto type = basetype();
  const auto funcName = myTokenizer::expect_ident();
  auto fn = std::make_unique<Function>();
  fn->name = funcName;

  //TODO: add function name to the current scope
  //new_gvar(funcName);
  
  myTokenizer::expect(myTokenizer::TokenType::PAREN_L);
  auto sc = enter_scope();
  fn->params = readFuncParams();

  if(myTokenizer::consume_symbol(myTokenizer::TokenType::SEMICOLON)){
    leave_scope();
    return nullptr;
  }
  
  myTokenizer::expect(myTokenizer::TokenType::BRACE_L);
  while(!myTokenizer::consume_symbol(myTokenizer::TokenType::BRACE_R)){
    fn->body.push_back(stmt());
  }
  leave_scope();
  fn->localVars = localVars;
  return fn;
}

  static std::unique_ptr<AstNode> new_desg_node2(const std::shared_ptr<Lunaria::Var>& lvar, std::list<std::unique_ptr<Designator>>& desg, std::list<std::unique_ptr<Designator>>::reverse_iterator iter){
    if(iter == desg.rend()){
      return new_var_node(lvar);
    }
    const int index = (*iter)->index;
    auto node = new_desg_node2(lvar, desg, ++iter);
    auto num = new_num(index);
    node = new_add(node, num);
    return new_unary(AstKind::AST_DEREF, node);
  }
  
  static std::unique_ptr<AstNode> new_desg_node(const std::shared_ptr<Lunaria::Var>& lvar, std::list<std::unique_ptr<Designator>>& desg, std::unique_ptr<AstNode>& rhs){
    auto lhs = new_desg_node2(lvar, desg, desg.rbegin());
    auto node = new_binary(AstKind::AST_ASSIGN, lhs, rhs);
    myParser::add_type(node);
    return node;
  }

  static void lvar_init_zero(std::list<std::unique_ptr<AstNode>>& cur, const std::shared_ptr<Lunaria::Var>& lvar, const std::shared_ptr<Lunaria::Type>& type, std::list<std::unique_ptr<Designator>>& desg){
    if(type->kind == Lunaria::TypeKind::ARRAY){
      for(int i = 0; i < type->array_size; i++){
	desg.push_back(std::make_unique<Designator>(i));
	lvar_init_zero(cur, lvar, type->base, desg);
	desg.pop_back();
      }
      return;
    } //if ARRAY

    auto num = new_num(0);
    cur.push_back(new_desg_node(lvar, desg, num));
    return;
  }

  static void lvar_initializer2(std::list<std::unique_ptr<AstNode>>& cur, const std::shared_ptr<Lunaria::Var>& lvar, const std::shared_ptr<Lunaria::Type>& type, std::list<std::unique_ptr<Designator>>& desg){
    auto& tok = myTokenizer::tokens.front();

    if(type->kind == Lunaria::TypeKind::ARRAY
       && type->base->kind == Lunaria::TypeKind::CHAR
       && tok->tokenType == myTokenizer::TokenType::STR){
      //char a[]="hoge"

      if(type->is_incomplete){
	//when the number of elements is omitted
	type->size = tok->literal.size();
	type->array_size = tok->literal.size();
	type->is_incomplete = false;
      }

      //compare array_size with string_length
      const int len = (type->array_size < tok->literal.size())
	? type->array_size : tok->literal.size();
      for(int i = 0; i < len; i++){
	desg.push_back(std::make_unique<Designator>(i));
	auto rhs = new_num(tok->literal[i]);
	cur.push_back(new_desg_node(lvar, desg, rhs));
	desg.pop_back();
      }

      //when type->array_size >= tok->str_len, fill the difference with 0
      for(int i = len; i < type->array_size; i++){
	desg.push_back(std::make_unique<Designator>(i));
	lvar_init_zero(cur, lvar, type->base, desg);
	desg.pop_back();
      }
      myTokenizer::tokens.pop_front();
      return;
    } //if ARRAY && CHAR && STR

    if(type->kind == Lunaria::TypeKind::ARRAY){
      //T a[] = {1, 2}
      const bool open = myTokenizer::consume_symbol(myTokenizer::TokenType::BRACE_L);
      //if the number of elements is omitted, any number of elements is allowed.
      const int limit = type->is_incomplete ? INT_MAX : type->array_size;
      int i = 0;
      if(!myTokenizer::look(myTokenizer::TokenType::BRACE_R)){
	do{
	  desg.push_back(std::make_unique<Designator>(i++));
	  lvar_initializer2(cur, lvar, type->base, desg);
	  desg.pop_back();
	}while(i < limit && !myTokenizer::look(myTokenizer::TokenType::BRACE_R) && myTokenizer::consume_symbol(myTokenizer::TokenType::COMMA));
      }

      if(open && !myTokenizer::consume_symbol(myTokenizer::TokenType::BRACE_R)){
	//The number of elements is exceeded
	//T a[1] = {1,2}
	skip_excess_elements();
      }

      //set array elements which is not initialized to zero
      //T a[3] = {1}
      while (i < type->array_size){
	desg.push_back(std::make_unique<Designator>(i++));
	lvar_init_zero(cur, lvar, type->base, desg);
	desg.pop_back();
      }

      if(type->is_incomplete){
	//the number of elements is omitted
	type->size = type->base->size * i;
	type->array_size = i;
	type->is_incomplete = false;
      }
      return;
    } //if ARRAY

    const bool open = myTokenizer::consume_symbol(myTokenizer::TokenType::BRACE_L);
    auto assign_node = assign();
    cur.push_back(new_desg_node(lvar, desg, assign_node));
    if(open){
      myTokenizer::expect(myTokenizer::TokenType::BRACE_R);
    }
    return;
  }
  
  static std::unique_ptr<AstNode> lvar_initializer(const std::shared_ptr<Lunaria::Var>& lvar){
    std::list<std::unique_ptr<AstNode>> body;
    std::list<std::unique_ptr<Designator>> desg = {};
    lvar_initializer2(body, lvar, lvar->type, desg);
    auto node = new_node(AstKind::AST_BLOCK);
    node->body = std::move(body);
    return node;
  }

//declaration = basetype ident type_suffix ("=" lvar_initializer)? ";"
//              | basetype ";"
  static std::unique_ptr<AstNode> declaration(){
    auto type = basetype();

    if(myTokenizer::consume_symbol(myTokenizer::TokenType::SEMICOLON)){
      return new_node(AstKind::AST_NULL);
    }
    
    const auto name = myTokenizer::expect_ident();
    type = type_suffix(type);

    if(type->kind == Lunaria::TypeKind::VOID){
      std::cerr << "variable is declared void\n";
      exit(1);
    }
    
    auto lvar = new_lvar(name, type);
    if(myTokenizer::consume_symbol(myTokenizer::TokenType::SEMICOLON)){
      if(type->is_incomplete){
	std::cerr << "incomplete type\n";
	exit(1);
      }
      return new_node(AstKind::AST_NULL);
    } //if ;
    
    myTokenizer::expect(myTokenizer::TokenType::ASSIGN);
    auto node = lvar_initializer(lvar);
    myTokenizer::expect(myTokenizer::TokenType::SEMICOLON);
    return node;
  }

  //type_suffix = ("[" const_expr "]" type_suffix)?
  static std::shared_ptr<Lunaria::Type> type_suffix(std::shared_ptr<Lunaria::Type>& type){
    if(!myTokenizer::consume_symbol(myTokenizer::TokenType::BRACKET_L)){
      return type;
    }
    
    int size = 0;
    bool is_incomplete = true;
    if(!myTokenizer::consume_symbol(myTokenizer::TokenType::BRACKET_R)){
      size = const_expr();
      is_incomplete = false;
      myTokenizer::expect(myTokenizer::TokenType::BRACKET_R);
    } //if
    
    type = type_suffix(type);
    if(type->is_incomplete){
      std::cerr << "incomplete type\n";
      exit(1);
    }
    type = Lunaria::array_of(type, size);
    type->is_incomplete = is_incomplete;
    return type;
  }

  //struct_member = basetype ident type-suffix ";"
  static std::shared_ptr<Lunaria::Member> struct_member(){
    auto type = basetype();
    const auto name = myTokenizer::expect_ident();
    type = type_suffix(type);
    myTokenizer::expect(myTokenizer::TokenType::SEMICOLON);

    auto mem = std::make_shared<Lunaria::Member>();
    mem->type = type;
    mem->name = name;
    return mem;
  }

  //struct_decl = "struct" ident? ("{" struct-member "}")?
  static std::shared_ptr<Lunaria::Type> struct_decl(){
    myTokenizer::expect(myTokenizer::TokenType::STRUCT);
    const auto tok = myTokenizer::consume_ident();

    //TODO: process tag

    if(!myTokenizer::consume_symbol(myTokenizer::TokenType::BRACE_L)){
      return Lunaria::struct_type();
    }

    std::list<std::shared_ptr<Lunaria::Member>> lst = {};
    while(!myTokenizer::consume_symbol(myTokenizer::TokenType::BRACE_R)){
      lst.push_back(struct_member());
    }
    auto type = Lunaria::struct_type();
    type->member = lst;

    //assign offset into the struct menbers
    int offset = 0;
    for(const auto& mem: type->member){
      if(mem->type->is_incomplete){
	std::cerr << "incomplete type in struct member\n";
	exit(1);
      }

      offset = Lunaria::align_to(offset, mem->type->align);
      mem->offset = offset;
      offset += mem->type->size;

      if(type->align < mem->type->align){
	//set type->align to max align in struct members
	type->align = mem->type->align;
      } //if
    } //for mem

    type->size = Lunaria::align_to(offset, type->align);
    type->is_incomplete = false;
    return type;
  }

  static long const_expr(){
    return eval(conditional());
  }

  static long eval(const std::unique_ptr<AstNode>& node){
    std::shared_ptr<Lunaria::Var> v = nullptr;
    std::shared_ptr<std::shared_ptr<Lunaria::Var>> vv = std::make_shared<std::shared_ptr<Lunaria::Var>>(v);
    return eval2(node, vv);
  }
  
  static long eval2(const std::unique_ptr<AstNode>& node,
		    std::shared_ptr<std::shared_ptr<Lunaria::Var>>& v){
    switch(node->kind){
    case AstKind::AST_ADD:
      return eval(node->lhs) + eval(node->rhs);
    case AstKind::AST_PTR_ADD:
      return eval2(node->lhs, v) + eval(node->rhs);
    case AstKind::AST_SUB:
      return eval(node->lhs) - eval(node->rhs);
    case AstKind::AST_PTR_SUB:
      return eval2(node->lhs, v) - eval(node->rhs);
    case AstKind::AST_PTR_DIFF:
      return eval2(node->lhs, v) - eval2(node->rhs, v);
    case AstKind::AST_MUL:
      return eval(node->lhs) * eval(node->rhs);
    case AstKind::AST_DIV:
      return eval(node->lhs) / eval(node->rhs);
    case AstKind::AST_REM:
      return eval(node->lhs) % eval(node->rhs);
    case AstKind::AST_BITOR:
      return eval(node->lhs) | eval(node->rhs);
    case AstKind::AST_BITXOR:
      return eval(node->lhs) ^ eval(node->rhs);
    case AstKind::AST_BITAND:
      return eval(node->lhs) & eval(node->rhs);
    case AstKind::AST_BITNOT:
      return ~eval(node->lhs);
    case AstKind::AST_SHL:
      return eval(node->lhs) << eval(node->rhs);
    case AstKind::AST_SHR:
      return eval(node->lhs) >> eval(node->rhs);
    case AstKind::AST_SAR:
      return eval(node->lhs) >> eval(node->rhs);
    case AstKind::AST_EQ:
      return eval(node->lhs) == eval(node->rhs);
    case AstKind::AST_NE:
      return eval(node->lhs) != eval(node->rhs);
    case AstKind::AST_LT:
      return eval(node->lhs) < eval(node->rhs);
    case AstKind::AST_LE:
      return eval(node->lhs) <= eval(node->rhs);
    case AstKind::AST_NOT:
      return !eval(node->lhs);
    case AstKind::AST_LOGOR:
      return eval(node->lhs) || eval(node->rhs);
    case AstKind::AST_LOGAND:
      return eval(node->lhs) && eval(node->rhs);
    case AstKind::AST_NUM:
      return node->val;
    case AstKind::AST_ADDR:
      if(!v || *v || node->lhs->kind != AstKind::AST_VAR || node->lhs->var->isLocal){
	std::cerr << "invalid initializer\n";
	exit(1);
      }
      *v = node->lhs->var;
      return 0;
    case AstKind::AST_VAR:
      if(!v || *v || node->var->type->kind != Lunaria::TypeKind::ARRAY){
	std::cerr << "invalid initializer\n";
	exit(1);
      }
      *v = node->var;
      return 0;
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
//       | "return" expr? ";"
//       | "if" "(" expr ")" stmt ("else" stmt)?
//       | "while" "(" expr ")" stmt
//       | "do" stmt "while" "(" expr ")" ";"
//       | "for" "(" (expr? ";" | declaration) expr? ";" expr? ")" stmt
//       | "break" ";"
//       | "continue" ";"
//       | "switch" "(" expr ")" stmt
//       | "case" const-expr ":" stmt
//       | "default" ":" stmt
//       | declaration
static std::unique_ptr<AstNode> stmt2(){
  std::unique_ptr<AstNode> node;
  
  //"return" expr? ";"
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::RETURN)){
    node = new_node(AstKind::AST_RETURN);
    if(myTokenizer::consume_symbol(myTokenizer::TokenType::SEMICOLON)){
      //"return" ";"
      node->lhs = nullptr;
      return node;
    }
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
    breaks.push(node->id);
    continues.push(node->id);
    
    myTokenizer::expect(myTokenizer::TokenType::PAREN_L);
    node->cond = expr();
    myTokenizer::expect(myTokenizer::TokenType::PAREN_R);
    node->then = stmt();

    breaks.pop();
    continues.pop();
    return node;
  }

  //"do" stmt "while" "(" expr ")" ";"
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::DO)){
    node = new_node(AstKind::AST_DO_WHILE);
    breaks.push(node->id);
    continues.push(node->id);
    
    node->then = stmt();
    myTokenizer::expect(myTokenizer::TokenType::WHILE);
    myTokenizer::expect(myTokenizer::TokenType::PAREN_L);
    node->cond = expr();
    myTokenizer::expect(myTokenizer::TokenType::PAREN_R);
    myTokenizer::expect(myTokenizer::TokenType::SEMICOLON);

    breaks.pop();
    continues.pop();
    return node;
  }

  //"for" "(" (expr? ";" | declaration) expr? ";" expr? ")" stmt
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::FOR)){
    node = new_node(AstKind::AST_FOR);
    breaks.push(node->id);
    continues.push(node->id);
    auto sc = enter_scope();
    
    myTokenizer::expect(myTokenizer::TokenType::PAREN_L);    
    if(!myTokenizer::consume_symbol(myTokenizer::TokenType::SEMICOLON)){
      //first expr
      if(isTypeName()){
	node->init = declaration();
      } else {
	node->init = expr();
	myTokenizer::expect(myTokenizer::TokenType::SEMICOLON);
      }
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

    breaks.pop();
    continues.pop();
    leave_scope();
    return node;
  }

  //"{" stmt* "}"
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::BRACE_L)){
    node = new_node(AstKind::AST_BLOCK);
    auto sc = enter_scope();
    while(!myTokenizer::consume_symbol(myTokenizer::TokenType::BRACE_R)){
      node->body.push_back(stmt());
    }
    leave_scope();
    return node;
  }

  //"break" ";"
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::BREAK)){
    if(breaks.empty()){
      std::cerr << "invalid break statement\n";
      exit(1);
    }
    myTokenizer::expect(myTokenizer::TokenType::SEMICOLON);
    node = new_node(AstKind::AST_BREAK);
    node->target = breaks.top();
    return node;
  }

  //"continue" ";"
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::CONTINUE)){
    if(continues.empty()){
      std::cerr << "invalid continue statement\n";
      exit(1);
    }
    myTokenizer::expect(myTokenizer::TokenType::SEMICOLON);
    node = new_node(AstKind::AST_CONTINUE);
    node->target = continues.top();
    return node;
  }

  //"switch" "(" expr ")" stmt
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::SWITCH)){
    node = new_node(AstKind::AST_SWITCH);
    //node->cases = {};

    myTokenizer::expect(myTokenizer::TokenType::PAREN_L);
    node->cond = expr();
    myTokenizer::expect(myTokenizer::TokenType::PAREN_R);

    breaks.push(node->id);
    switches.push(node->id);
    
    node->body.push_back(stmt());

    breaks.pop();
    switches.pop();
    return node;
  }
  
  //"case" const-expr ":" stmt
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::CASE)){
    if(switches.empty()){
      std::cerr << "invalid case statement\n";
      exit(1);
    }
    node = new_node(AstKind::AST_CASE);
    const long long val = const_expr();
    myTokenizer::expect(myTokenizer::TokenType::COLON);

    node->body.push_back(stmt());
    node->val = val;

    node->_switch = switches.top();
    return node;
  }
  
  //"default" ":" stmt
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::DEFAULT)){
    if(switches.empty()){
      std::cerr << "invalid case statement\n";
      exit(1);
    }
    myTokenizer::expect(myTokenizer::TokenType::COLON);
    node = new_node(AstKind::AST_DEFAULT);
    node->body.push_back(stmt());

    node->_switch = switches.top();
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
  
//assign = conditional (assign-op assign)?
//assign-op = "=" | "+=" | "-=" | "*=" | "/="
static std::unique_ptr<AstNode> assign(){
  auto node = conditional();
  add_type(node);
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::ASSIGN)){
    auto node_assign = assign();
    return new_binary(AstKind::AST_ASSIGN, node, node_assign);
  }

  if(myTokenizer::consume_symbol(myTokenizer::TokenType::PLUS_ASSIGN)){
    auto node_assign = assign();
    return new_binary(AstKind::AST_ADD_ASSIGN, node, node_assign);
  } //if +=

  if(myTokenizer::consume_symbol(myTokenizer::TokenType::MINUS_ASSIGN)){
    auto node_assign = assign();
    return new_binary(AstKind::AST_SUB_ASSIGN, node, node_assign);
  } //if -=

  if(myTokenizer::consume_symbol(myTokenizer::TokenType::STAR_ASSIGN)){
    auto node_assign = assign();
    return new_binary(AstKind::AST_MUL_ASSIGN, node, node_assign);
  } //if *=

  if(myTokenizer::consume_symbol(myTokenizer::TokenType::SLASH_ASSIGN)){
    auto node_assign = assign();
    return new_binary(AstKind::AST_DIV_ASSIGN, node, node_assign);
  } //if /=
  return node;
}

  //conditional = logor ("?" expr ":" conditional)?
  static std::unique_ptr<AstNode> conditional(){
    auto node = logor();
    if(myTokenizer::consume_symbol(myTokenizer::TokenType::QUESTION)){
      auto node_cond = new_node(AstKind::AST_CONDITIONAL);
      node_cond->cond = std::move(node);
      node_cond->then = expr();
      expect(myTokenizer::TokenType::COLON);
      node_cond->els = conditional();
      return node_cond;
    }
    return node;
  }
  
  //logor = logand ("||" logand)*
  static std::unique_ptr<AstNode> logor(){
    auto node = logand();
    while(myTokenizer::consume_symbol(myTokenizer::TokenType::OROR)){
      auto node_logand = logand();
      node = new_binary(AstKind::AST_LOGOR, node, node_logand);
    }
    return node;
  }

  //logand = bitor ("&&" bitor)*
  static std::unique_ptr<AstNode> logand(){
    auto node = _bit_or();
    while(myTokenizer::consume_symbol(myTokenizer::TokenType::ANDAND)){
      auto node_or = _bit_or();
      node = new_binary(AstKind::AST_LOGAND, node, node_or);
    }
    return node;
  }

  //bitor = birxor ("|" bitxor)*
  static std::unique_ptr<AstNode> _bit_or(){
    auto node = _bit_xor();
    while(myTokenizer::consume_symbol(myTokenizer::TokenType::OR)){
      auto tmp = _bit_xor();
      node = new_binary(AstKind::AST_BITOR, node, tmp);
    }
    return node;
  }

  //bitxor = bitand ("^" bitand)*
  static std::unique_ptr<AstNode> _bit_xor(){
    auto node = _bit_and();
    while(myTokenizer::consume_symbol(myTokenizer::TokenType::CARET)){
      auto tmp = _bit_and();
      node = new_binary(AstKind::AST_BITXOR, node, tmp);
    }
    return node;
  }

  //bitand = equality ("&" equality)*
  static std::unique_ptr<AstNode> _bit_and(){
    auto node = equality();
    while(myTokenizer::consume_symbol(myTokenizer::TokenType::AND)){
      auto tmp = equality();
      node = new_binary(AstKind::AST_BITAND, node, tmp);
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

//relational = shift ("<" shift | "<=" shift | ">" shift | ">=" shift)*
static std::unique_ptr<AstNode> relational(){
  auto node = shift();
  while(1){
    if(myTokenizer::consume_symbol(myTokenizer::TokenType::LT)){
      auto node_sht = shift();
      node = new_binary(AstKind::AST_LT, node, node_sht);
    } else if(myTokenizer::consume_symbol(myTokenizer::TokenType::LE)){
      auto node_sht = shift();
      node = new_binary(AstKind::AST_LE, node, node_sht);
    } else if(myTokenizer::consume_symbol(myTokenizer::TokenType::GT)){
      auto node_sht = shift();
      node = new_binary(AstKind::AST_LT, node_sht, node);
    } else if(myTokenizer::consume_symbol(myTokenizer::TokenType::GE)){
      auto node_sht = shift();
      node = new_binary(AstKind::AST_LE, node_sht, node);
    } else {
      return node;
    }
  }
}
  //shift = add ("<<" add || ">>" add)*
  static std::unique_ptr<AstNode> shift(){
    auto node = add();
    while(1){
      if(myTokenizer::consume_symbol(myTokenizer::TokenType::SHL)){
	auto node_add = add();
	node = new_binary(AstKind::AST_SHL, node, node_add);
      } else if(myTokenizer::consume_symbol(myTokenizer::TokenType::SHR)){
	auto node_add = add();
	//TODO: now, node is absolutely signed,
	//so that the kind is AST_SAR 
	node = new_binary(AstKind::AST_SAR, node, node_add);
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
      node = new_add(node, node_mul);
    } else if(myTokenizer::consume_symbol(myTokenizer::TokenType::MINUS)){
      auto node_mul = mul();
      node = new_sub(node, node_mul);
    } else {
      return node;
    }
  }
}

//mul = unary ("*" unary | "/" unary | "%" unary)*
static std::unique_ptr<AstNode> mul(){
  auto node = unary();

  while(1){
    if(myTokenizer::consume_symbol(myTokenizer::TokenType::STAR)){
      auto node_unary = unary();
      node = new_binary(AstKind::AST_MUL, node, node_unary);
    } else if(myTokenizer::consume_symbol(myTokenizer::TokenType::SLASH)){
      auto node_unary = unary();
      node = new_binary(AstKind::AST_DIV, node, node_unary);
    } else if(myTokenizer::consume_symbol(myTokenizer::TokenType::PERCENT)){
      auto node_unary = unary();
      node = new_binary(AstKind::AST_REM, node, node_unary);
    } else {
      return node;
    }
  }
}

  static std::unique_ptr<AstNode>
  new_unary(AstKind k, std::unique_ptr<AstNode>& lhs){
    auto node = new_node(k);
    node->lhs = std::move(lhs);
    return node;
  }

//unary = ("+" | "-" | "*" | "&" | "!" | "~")? unary
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
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::NOT)){
    auto node_not = new_node(AstKind::AST_NOT);
    node_not->lhs = unary();
    return node_not;
  }
  if(myTokenizer::consume_symbol(myTokenizer::TokenType::TILDA)){
    auto node = new_node(AstKind::AST_BITNOT);
    node->lhs = unary();
    return node;
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
  
  static std::shared_ptr<Lunaria::Member> find_member(const std::shared_ptr<Lunaria::Type>& type, const std::string& name){
    for(const auto& mem: type->member){
      if(mem->name == name){
	return mem;
      }
    }
    return nullptr;
  }

  static std::unique_ptr<AstNode> struct_ref(std::unique_ptr<AstNode>& lhs){
    add_type(lhs);
    if(lhs->type->kind != Lunaria::TypeKind::STRUCT){
      std::cerr << "lhs is not a struct type\n";
      exit(1);
    }

    const auto mem = find_member(lhs->type, myTokenizer::expect_ident());
    if(!mem){
      std::cerr << "no such member\n";
      exit(1);
    }

    auto node = new_unary(AstKind::AST_MEMBER, lhs);
    node->member = mem;
    return node;
  }

  //postfix = primary ("[" expr "]" | "." ident | "->" ident | "++" | "--")*
  static std::unique_ptr<AstNode> postfix(){
    auto node = primary();
    while(true){
      if(myTokenizer::consume_symbol(myTokenizer::TokenType::BRACKET_L)){
	//a[b] => AST_SUBSCRIPTED, lhs=a, rhs=b
	auto node_expr = expr();
	expect(myTokenizer::TokenType::BRACKET_R);
	node = new_binary(AstKind::AST_SUBSCRIPTED, node, node_expr);
	continue;
      }

      if(myTokenizer::consume_symbol(myTokenizer::TokenType::DOT)){
	node = struct_ref(node);
	continue;
      }

      if(myTokenizer::consume_symbol(myTokenizer::TokenType::ARROW)){
	//x->y ==> (*x).y
	node = new_unary(AstKind::AST_DEREF, node);
	node = struct_ref(node);
	continue;
      }

      if(myTokenizer::consume_symbol(myTokenizer::TokenType::PLUSPLUS)){
	node = new_unary(AstKind::AST_POST_INC, node);
	continue;
      }

      if(myTokenizer::consume_symbol(myTokenizer::TokenType::MINUSMINUS)){
	node = new_unary(AstKind::AST_POST_DEC, node);
	continue;
      }
      
      break;
    } //while
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
      add_type(node);
      return node;
    }

    //variable
    /*
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
    */
    auto vsc = find_var_scope(token);
    if(vsc){
      if(vsc->var){
	return new_var_node(vsc->var);
      }
    }
    std::cerr << "undefined variable\n";
    exit(1);
    //return node;
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
