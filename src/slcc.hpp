#ifndef SLCC_H
#define SLCC_H

//#include "lunaria.hpp"



//--------------------
//Tokenizer
namespace Selene::Tokenizer {
enum class TokenType {
  NUM, //number
  PLUS, //+
  MINUS, //-
  STAR, //*
  SLASH, // /
  PERCENT, //%
  LT, //<
  LE, //<=
  SHL, //<<
  GT, //>
  GE, //>=
  SHR, //>>
  EQ, //==
  ASSIGN, //=
  NE, //!=
  NOT, //!
  PAREN_L, //(
  PAREN_R, //)
  BRACE_L, //{
  BRACE_R, //}
  BRACKET_L, //[
  BRACKET_R, //]
  COLON, //:
  SEMICOLON, //;
  IDENT, //identifier
  RETURN, //return
  IF, //if
  ELSE, //else
  WHILE, //while
  DO, //do
  FOR, //for
  BREAK, //break
  CONTINUE, //continue
  SWITCH, //switch
  CASE, //case
  DEFAULT, //default
  COMMA, //,
  AND, //&
  ANDAND, //&&
  OR, //|
  OROR, //||
  INT, //int
  CHAR, //char
  SHORT, //short
  LONG, //long
  VOID, //void
  SIZEOF, //sizeof
  STR, //string literal
  PLUSPLUS, //++
  MINUSMINUS, //--
  PLUS_ASSIGN, //+=
  MINUS_ASSIGN, //-=
  STAR_ASSIGN, //*=
  SLASH_ASSIGN, // /=
  CARET, //^
  TILDA, //~
  QUESTION, //?
  STRUCT, 
  DOT, //.
  ARROW, //->
  TK_EOF,
};

struct Token {                   
  TokenType tokenType;
  unsigned long long value; //available when tokenType is NUM
  std::string str;
  std::string literal; //string literal
  Token(TokenType _tokenType, unsigned long long _value, const std::string& _str, const std::string& _literal) 
    : tokenType(_tokenType), value(_value), str(_str), literal(_literal)
  {}
};

extern std::deque<std::unique_ptr<Token>> tokens;

void expect(TokenType tk_type);
unsigned long long expect_number();
std::string expect_ident();
bool consume_symbol(TokenType tk_type);
std::unique_ptr<Token> consume_ident();
std::unique_ptr<Token> consume_str();
bool look(TokenType tk_type);
bool at_eof();
void tokenize(const std::string& input);
} //namespace Selene::Tokenizer

//-------------------
//Parser
namespace Selene::Parser {
enum class AstKind {
  AST_NUM,
  AST_ADD,
  AST_SUB,
  AST_MUL,
  AST_DIV,
  AST_REM,
  AST_LT, //<
  AST_LE, //<=
  AST_EQ, //==
  AST_NE, //!=
  AST_ASSIGN, //=
  AST_VAR, //local or global variable
  AST_RETURN, //return
  AST_IF, //if
  AST_WHILE, //while
  AST_DO_WHILE, //do-while
  AST_FOR, //for
  AST_BREAK, //break
  AST_CONTINUE, //continue
  AST_SWITCH, //switch
  AST_CASE, //case
  AST_DEFAULT, //default
  AST_BLOCK, //{}
  AST_FUNCALL, //function call
  AST_DEREF, //*
  AST_ADDR, //&
  AST_PTR_ADD,
  AST_PTR_SUB,
  AST_PTR_DIFF,
  AST_SUBSCRIPTED, //a[i]
  AST_PRE_INC, //++i
  AST_PRE_DEC, //--i
  AST_POST_INC, //i++
  AST_POST_DEC, //i--
  AST_LOGOR, //||
  AST_LOGAND, //&&
  AST_NOT, //!
  AST_SHL, //<< logical
  AST_SHR, //>> logical
  AST_SAR, //>> arith
  AST_ADD_ASSIGN, //+=
  AST_SUB_ASSIGN, //-=
  AST_MUL_ASSIGN, //*=
  AST_DIV_ASSIGN, // /=
  AST_BITOR, //|
  AST_BITXOR, //^
  AST_BITAND, //&
  AST_BITNOT, //~
  AST_CONDITIONAL, //? :
  AST_MEMBER, //.
  AST_CAST, 
  AST_NULL,
};

struct AstNode {
  int id;
  AstKind kind;
  std::unique_ptr<AstNode> lhs;
  std::unique_ptr<AstNode> rhs;
  long long val;

  std::shared_ptr<Lunaria::Var> var;

  std::unique_ptr<AstNode> cond; //if,while,for
  std::unique_ptr<AstNode> then; //if,while,for
  std::unique_ptr<AstNode> els; //if
  std::unique_ptr<AstNode> init; //for
  std::unique_ptr<AstNode> inc; //for

  int target; //break, continue

  //switch, case, default
  //std::vector<int> cases;
  int _switch;
  
  std::list<std::unique_ptr<AstNode>> body;

  std::string funcName; //function name
  std::list<std::unique_ptr<AstNode>> args;

  std::shared_ptr<Lunaria::Type> type;

  std::shared_ptr<Lunaria::Member> member;
};

struct Function {
  std::string name;
  std::list<std::shared_ptr<Lunaria::Var>> params;
  std::list<std::unique_ptr<AstNode>> body;
  std::unordered_set<std::shared_ptr<Lunaria::Var>,Lunaria::VarSharedPtrHash,Lunaria::VarSharedPtrEqual> localVars;
};

struct Program {
  std::list<std::unique_ptr<Function>> fns;
  //std::unordered_map<std::string, std::shared_ptr<Lunaria::Var>> globalVars;
  std::unordered_set<std::shared_ptr<Lunaria::Var>,Lunaria::VarSharedPtrHash,Lunaria::VarSharedPtrEqual> globalVars;
};

  struct Designator{
    int index; //for array
    std::shared_ptr<Lunaria::Member> member;
    Designator(int i): index(i), member(nullptr)
    {}
    Designator(int i, const std::shared_ptr<Lunaria::Member>& m): index(i), member(m)
    {}
  };

  std::unique_ptr<Program> program();
  void add_type(std::unique_ptr<AstNode>& node);
} //namespace Selene::Parser


#endif
