#ifndef YCC_H
#define YCC_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <deque>
#include <memory>
#include <cassert>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <climits>
#include <stack>
#include <format>

//---------------------------
//Lunaria Utility
namespace Lunaria {
  enum class TypeKind {
    INT,
    CHAR,
    SHORT,
    LONG,
    VOID,
    PTR,
    ARRAY,
  };

  struct Type {
    TypeKind kind;
    std::shared_ptr<Type> base;
    int size;
    int align;
    int array_size;
    bool is_incomplete; //whether index is omitted
    
    Type(){}
    Type(TypeKind k, int sz, int al)
      : kind(k), base(nullptr), size(sz), align(al),
	array_size(0), is_incomplete(false){}
    Type(TypeKind k, const std::shared_ptr<Type>& bs, int sz, int al)
      : kind(k), base(bs), size(sz), align(al),
	array_size(0), is_incomplete(false){}
  };

  extern std::shared_ptr<Type> int_type;
  extern std::shared_ptr<Type> char_type;
  extern std::shared_ptr<Type> short_type;
  extern std::shared_ptr<Type> long_type;
  extern std::shared_ptr<Type> void_type;
  bool is_integer(const std::shared_ptr<Type>& type);
  std::shared_ptr<Type> pointer_to(const std::shared_ptr<Type>&);
  int align_to(int n, int align);
  std::shared_ptr<Type> array_of(const std::shared_ptr<Type>&, int size);

  struct Initializer {
    int size;
    int val;

    //reference to other global variable
    std::string label;
    long addend;

    Initializer(int sz, int v)
      : size(sz), val(v), label(""), addend(0)
    {}
    Initializer(const std::string& _label, long _addend)
      : size(0), val(0), label(_label), addend(_addend)
    {}
  };
  
  struct Var {
    int id;
    std::string name;
    int offset; //for local variable
    std::shared_ptr<Type> type;
    bool isLocal;
    bool isLiteral;
    std::string literal; //for global variable
    std::vector<std::unique_ptr<Initializer>> initializer;
    Var(const std::string& _name, const std::shared_ptr<Lunaria::Type>& _type, bool _isLocal)
      : id(0), name(_name), offset(0),
	type(_type), isLocal(_isLocal), isLiteral(false),
	literal(""), initializer()
    {}
  };

  struct VarSharedPtrHash {
    size_t operator()(const std::shared_ptr<Var>& p) const {
        if (!p) return 0; 
        return std::hash<int>{}(p->id);
    }
};
  
  struct VarSharedPtrEqual {
    bool operator()(const std::shared_ptr<Var>& a, const std::shared_ptr<Var>& b) const {
        if (!a && !b) return true;
        if (!a || !b) return false;
        return a->id == b->id;
    }
  };
} //namespace Lunaria

//--------------------
//Tokenizer
namespace myTokenizer {
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
int expect_number();
std::string expect_ident();
bool consume_symbol(TokenType tk_type);
std::unique_ptr<Token> consume_ident();
std::unique_ptr<Token> consume_str();
bool look(TokenType tk_type);
bool at_eof();
void tokenize(const std::string& input);
} //namespace myTokenizer

//-------------------
//Parser
namespace myParser {
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
};

struct Function {
  std::string name;
  std::list<std::shared_ptr<Lunaria::Var>> params;
  std::list<std::unique_ptr<AstNode>> body;
  //std::unordered_map<std::string, std::shared_ptr<Lunaria::Var>> localVars;
  std::unordered_set<std::shared_ptr<Lunaria::Var>,Lunaria::VarSharedPtrHash,Lunaria::VarSharedPtrEqual> localVars;
};

struct Program {
  std::list<std::unique_ptr<Function>> fns;
  //std::unordered_map<std::string, std::shared_ptr<Lunaria::Var>> globalVars;
  std::unordered_set<std::shared_ptr<Lunaria::Var>,Lunaria::VarSharedPtrHash,Lunaria::VarSharedPtrEqual> globalVars;
};

  struct Designator{
    int index; //for array
    Designator(int i): index(i){}
  };

  std::unique_ptr<Program> program();
  void add_type(std::unique_ptr<AstNode>& node);
} //namespace myParser

//---------------------------
//HIR
namespace myLIR {
    struct BasicBlock;
  }

namespace myHIR {
enum class HirKind {
  HIR_IMM,
  HIR_ADD,
  HIR_SUB,
  HIR_MUL,
  HIR_DIV,
  HIR_REM,
  HIR_LT, //<
  HIR_LE, //<=
  HIR_EQ, //==
  HIR_NE, //!=
  HIR_ASSIGN, //=
  HIR_VAR, //local or global variable
  HIR_RETURN,
  HIR_IF, //if
  HIR_WHILE, //while
  HIR_DO_WHILE, //do-while
  HIR_FOR, //for
  HIR_BREAK, //break
  HIR_CONTINUE, //continue
  HIR_SWITCH, //switch
  HIR_CASE, //case
  HIR_DEFAULT, //default
  HIR_BLOCK, //{}
  HIR_FUNCALL, //function call
  HIR_DEREF, //*
  HIR_ADDR, //&
  HIR_PTR_ADD,
  HIR_PTR_SUB,
  HIR_PTR_DIFF,
  HIR_SUBSCRIPTED,
  HIR_PRE_INC, //++i
  HIR_PRE_DEC, //--i
  HIR_POST_INC, //i++
  HIR_POST_DEC, //i--
  HIR_LOGOR, //||
  HIR_LOGAND, //&&
  HIR_NOT, //!
  HIR_SHL, //<< logical
  HIR_SHR, //>> logical
  HIR_SAR, //>> arith
  HIR_ADD_ASSIGN, //+=
  HIR_SUB_ASSIGN, //-=
  HIR_MUL_ASSIGN, //*=
  HIR_DIV_ASSIGN, // /=
  HIR_BITOR, //|
  HIR_BITXOR, //^
  HIR_BITAND, //&
  HIR_BITNOT, //~
  HIR_CONDITIONAL, //? :
  HIR_NULL,
};
  
struct HirNode {
  HirKind kind;
  std::shared_ptr<HirNode> lhs;
  std::shared_ptr<HirNode> rhs;
  long long val;

  std::shared_ptr<Lunaria::Var> var;

  std::shared_ptr<HirNode> cond; //if,while,for
  std::shared_ptr<HirNode> then; //if,while,for
  std::shared_ptr<HirNode> els; //if
  std::shared_ptr<HirNode> init; //for
  std::shared_ptr<HirNode> inc; //for

  //break and continue
  std::shared_ptr<HirNode> target; 
  std::shared_ptr<myLIR::BasicBlock> _break;
  std::shared_ptr<myLIR::BasicBlock> _continue;

  std::vector<std::shared_ptr<HirNode>> cases;
  std::shared_ptr<HirNode> _default;
  std::shared_ptr<myLIR::BasicBlock> _case_bb;

  std::list<std::shared_ptr<HirNode>> body;

  std::string funcName; //function name
  std::list<std::shared_ptr<HirNode>> args;

  std::shared_ptr<Lunaria::Type> type;
};

  struct Function {
    std::string name;
    std::list<std::shared_ptr<Lunaria::Var>> params;
    std::list<std::shared_ptr<HirNode>> body;
    //std::unordered_map<std::string, std::shared_ptr<Lunaria::Var>> localVars;
    std::unordered_set<std::shared_ptr<Lunaria::Var>,Lunaria::VarSharedPtrHash,Lunaria::VarSharedPtrEqual> localVars;
  };

  struct Program {
    std::list<std::unique_ptr<Function>> fns;
    //std::unordered_map<std::string, std::shared_ptr<Lunaria::Var>> globalVars;
    std::unordered_set<std::shared_ptr<Lunaria::Var>,Lunaria::VarSharedPtrHash,Lunaria::VarSharedPtrEqual> globalVars;
  };

  std::shared_ptr<HirNode> new_node(HirKind kind, int ast_id = -1);
  std::shared_ptr<HirNode> new_binary(HirKind kind,
				      std::shared_ptr<HirNode>& lhs,
				      std::shared_ptr<HirNode>& rhs);
  std::shared_ptr<HirNode> new_num(long long i);
  std::shared_ptr<HirNode> new_var_node(const std::shared_ptr<Lunaria::Var>& var);
  std::shared_ptr<HirNode> new_add(std::shared_ptr<HirNode>& lhs,
				   std::shared_ptr<HirNode>& rhs);
  std::shared_ptr<HirNode> new_sub(std::shared_ptr<HirNode>& lhs,
				   std::shared_ptr<HirNode>& rhs);
std::unique_ptr<Program>
  generateHirNode(const std::unique_ptr<myParser::Program>&);
  void add_type(std::shared_ptr<HirNode>& node);
} //namespace myHIR
//---------------------------
//LIR
namespace myLIR {
enum class LirKind {
  LIR_REG,
  LIR_MOV,
  LIR_IMM,
  LIR_ADD,
  LIR_SUB,
  LIR_MUL,
  LIR_DIV,
  LIR_REM,
  LIR_LT, //<
  LIR_LE, //<=
  LIR_EQ, //==
  LIR_NE, //!=
  LIR_LVAR,
  LIR_LOAD,
  LIR_STORE,
  LIR_STORE_ARG,
  LIR_RETURN,
  LIR_BR, //branch
  LIR_JMP, //jump
  LIR_FUNCALL,
  LIR_PTR_ADD,
  LIR_PTR_SUB,
  LIR_PTR_DIFF,
  LIR_LABEL_ADDR, //for global variable
  LIR_SHL, //<< logical
  LIR_SHR, //>> logical
  LIR_SAR, //>> arith
  LIR_BITOR, //|
  LIR_BITXOR, //^
  LIR_BITAND, //&
  LIR_NULL,
};

struct LirNode;
struct BasicBlock {
  int label;
  std::list<std::shared_ptr<LirNode>> insts;
  std::list<std::shared_ptr<BasicBlock>> pred;
  std::list<std::shared_ptr<BasicBlock>> succ;
  std::shared_ptr<LirNode> param;
};

struct LirNode {
  LirKind opcode; //d = a <opcode> b
  std::shared_ptr<LirNode> d; //destination operand
  std::shared_ptr<LirNode> a; //left source operand
  std::shared_ptr<LirNode> b; //right source operand
  long long imm;

  int vn; //virtual register number
  int rn; //real register number
  int def;
  int lastUse;

  std::shared_ptr<Lunaria::Var> lvar;
  std::string name; //for global variable

  std::shared_ptr<BasicBlock> bb1;
  std::shared_ptr<BasicBlock> bb2;
  std::shared_ptr<LirNode> bbarg;

  std::string funcName;
  std::vector<std::shared_ptr<LirNode>> args;

  int type_size;
  int type_base_size;

  LirNode(): opcode(LirKind::LIR_NULL), d(nullptr),
	     a(nullptr), b(nullptr), imm(-1),
	     vn(-1), rn(-1), def(0), lastUse(0),
	     lvar(nullptr), name(""),
	     bb1(nullptr), bb2(nullptr), bbarg(nullptr),
	     funcName(""), args({}),
	     type_size(0), type_base_size(0)
  {}
};

  struct Function {
    std::string name;
    std::list<std::shared_ptr<Lunaria::Var>> params;
    //std::unordered_map<std::string, std::shared_ptr<Lunaria::Var>> localVars;
    std::unordered_set<std::shared_ptr<Lunaria::Var>,Lunaria::VarSharedPtrHash,Lunaria::VarSharedPtrEqual> localVars;
    int stackSize;
    std::list<std::shared_ptr<BasicBlock>> bbs;
  };

  struct Program {
    std::list<std::shared_ptr<Function>> fns;
    //std::unordered_map<std::string, std::shared_ptr<Lunaria::Var>> globalVars;
    std::unordered_set<std::shared_ptr<Lunaria::Var>,Lunaria::VarSharedPtrHash,Lunaria::VarSharedPtrEqual> globalVars;
  };

std::unique_ptr<Program>
generateLirNode(const std::unique_ptr<myHIR::Program>&);
} //namespace myLIR

//---------------------------
//Register Allocation
namespace myRegAlloc {
void allocateRegister_x86_64(std::unique_ptr<myLIR::Program>&);
}

//---------------------------
//Code Generation
namespace myCodeGen {
void gen_x86_64(const std::unique_ptr<myLIR::Program>&);
}

#endif
