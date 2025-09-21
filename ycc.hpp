#ifndef YCC_H
#define YCC_H

#include <iostream>
#include <string>
#include <queue>
#include <memory>
#include <cassert>
#include <list>
#include <unordered_map>
#include <vector>

//--------------------
//Tokenizer
enum class TokenType {
  NUM, //number
  PLUS, //+
  MINUS, //-
  STAR, //*
  SLASH, // /
  LT, //<
  LE, //<=
  GT, //>
  GE, //>=
  EQ, //==
  ASSIGN, //=
  NE, //!=
  NOT, //!
  PAREN_L, //(
  PAREN_R, //)
  SEMICOLON, //;
  IDENT, //identifier
  RETURN, //return
  IF, //if
  ELSE, //else
  WHILE, //while
  FOR, //for
  TK_EOF,
};

struct Token {                   
  TokenType tokenType;
  int value; //available when tokenType is NUM
  std::string str;
  Token(TokenType _tokenType, int _value, std::string _str) 
    : tokenType(_tokenType), value(_value), str(_str)
  {}                                                   
};

extern std::queue<std::unique_ptr<Token>> tokens;

void expect(TokenType tk_type);
int expect_number();
bool consume_symbol(TokenType tk_type);
std::unique_ptr<Token> consume_ident();
bool at_eof();
void tokenize(const std::string& input);

//-------------------
//Parser
enum class AstKind {
  AST_NUM,
  AST_ADD,
  AST_SUB,
  AST_MUL,
  AST_DIV,
  AST_LT, //<
  AST_LE, //<=
  AST_EQ, //==
  AST_NE, //!=
  AST_ASSIGN, //=
  AST_LVAR, //local variable
  AST_RETURN, //return
  AST_IF, //if
  AST_WHILE, //while
  AST_FOR, //for
};

struct LVar {
  std::string name;
  int offset;
};

struct AstNode {
  AstKind kind;
  std::unique_ptr<AstNode> lhs;
  std::unique_ptr<AstNode> rhs;
  int val;

  std::shared_ptr<LVar> lvar;

  std::unique_ptr<AstNode> cond; //if,while,for
  std::unique_ptr<AstNode> then; //if,while,for
  std::unique_ptr<AstNode> els; //if
  std::unique_ptr<AstNode> init; //for
  std::unique_ptr<AstNode> inc; //for
};
extern std::unordered_map<std::string, std::shared_ptr<LVar>> localVars;
std::list<std::unique_ptr<AstNode>> program();


//---------------------------
//HIR
enum class HirKind {
  HIR_IMM,
  HIR_ADD,
  HIR_SUB,
  HIR_MUL,
  HIR_DIV,
  HIR_LT, //<
  HIR_LE, //<=
  HIR_EQ, //==
  HIR_NE, //!=
  HIR_ASSIGN, //=
  HIR_LVAR, //local variable
  HIR_RETURN,
  HIR_IF, //if
  HIR_WHILE, //while
  HIR_FOR, //for
};

struct HirNode {
  HirKind kind;
  std::unique_ptr<HirNode> lhs;
  std::unique_ptr<HirNode> rhs;
  int val;

  std::shared_ptr<LVar> lvar;

  std::unique_ptr<HirNode> cond; //if,while,for
  std::unique_ptr<HirNode> then; //if,while,for
  std::unique_ptr<HirNode> els; //if
  std::unique_ptr<HirNode> init; //for
  std::unique_ptr<HirNode> inc; //for
};

std::list<std::unique_ptr<HirNode>> generateHirNode(const std::list<std::unique_ptr<AstNode>>&);
  
//---------------------------
//LIR
enum class LirKind: int {
  LIR_REG,
  LIR_MOV,
  LIR_IMM,
  LIR_ADD,
  LIR_SUB,
  LIR_MUL,
  LIR_DIV,
  LIR_LT, //<
  LIR_LE, //<=
  LIR_EQ, //==
  LIR_NE, //!=
  LIR_LVAR,
  LIR_LOAD,
  LIR_STORE,
  LIR_RETURN,
  LIR_BR, //branch
  LIR_JMP, //jump
  LIR_NULL,
};

struct LirNode;
struct BasicBlock {
  int label;
  std::list<std::shared_ptr<LirNode>> insts;
  std::list<std::shared_ptr<BasicBlock>> pred;
  std::list<std::shared_ptr<BasicBlock>> succ;
};

struct LirNode {
  LirKind opcode; //d = a <opcode> b
  std::shared_ptr<LirNode> d; //destination operand
  std::shared_ptr<LirNode> a; //left source operand
  std::shared_ptr<LirNode> b; //right source operand
  int imm;

  int vn; //virtual register number
  int rn; //real register number
  int def;
  int lastUse;

  std::shared_ptr<LVar> lvar;

  std::shared_ptr<BasicBlock> bb1;
  std::shared_ptr<BasicBlock> bb2;

  LirNode(): opcode(LirKind::LIR_NULL), d(nullptr),
	     a(nullptr), b(nullptr), imm(-1),
	     vn(-1), rn(-1), def(0), lastUse(0),
	     lvar(nullptr)
  {}
};

//extern std::list<std::shared_ptr<BasicBlock>> BBList;
std::list<std::shared_ptr<BasicBlock>>
generateLirNode(const std::list<std::unique_ptr<HirNode>>&);

//---------------------------
//Register Allocation
void allocateRegister_x86_64(std::list<std::shared_ptr<BasicBlock>>& bbList);

//---------------------------
//Code Generation
extern std::string regs[7];
void gen_x86_64(const std::list<std::shared_ptr<BasicBlock>>& bbList);

#endif
