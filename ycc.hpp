#ifndef YCC_H
#define YCC_H

#include <iostream>
#include <string>
#include <queue>
#include <memory>
#include <cassert>
#include <list>

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
  TK_EOF,
};

struct Token {                   
  TokenType tokenType;                                     
  int value; //available when tokenType is NUM
  Token(TokenType _tokenType, int _value) 
    : tokenType(_tokenType), value(_value)                
  {}                                                   
};

extern std::queue<std::unique_ptr<Token>> tokens;

void expect(TokenType tk_type);
int expect_number();
bool consume_symbol(TokenType tk_type);
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
};

struct AstNode {
  AstKind kind;
  std::unique_ptr<AstNode> lhs;
  std::unique_ptr<AstNode> rhs;
  int val;
};

std::unique_ptr<AstNode> program();


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
};

struct HirNode {
  HirKind kind;
  std::unique_ptr<HirNode> lhs;
  std::unique_ptr<HirNode> rhs;
  int val;  
};

  std::unique_ptr<HirNode> generateHirNode(const std::unique_ptr<AstNode>&);
  
//---------------------------
//LIR
enum class LirKind {
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
  LIR_NULL,
};

struct LirNode {
  LirKind opcode; //d = a <opcode> b
  std::shared_ptr<LirNode> d; //destination operand
  std::shared_ptr<LirNode> a; //left source operand
  std::shared_ptr<LirNode> b; //right source operand
  int imm;

  //The following two variables are valid when LIR_REG.
  int vn; //virtual register number
  int rn; //real register number
  //----------------------------

  LirNode(): opcode(LirKind::LIR_NULL), d(nullptr),
	     a(nullptr), b(nullptr), imm(-1),
	     vn(-1), rn(-1)
  {}
};

std::list<std::shared_ptr<LirNode>>
generateLirNode(const std::unique_ptr<HirNode>&);

//---------------------------
//Register Allocation
void allocateRegister_x86_64(std::list<std::shared_ptr<LirNode>>& lirList);

//---------------------------
//Code Generation
extern std::string regs[7];
void gen_x86_64(const std::list<std::shared_ptr<LirNode>>& lirList);

#endif
