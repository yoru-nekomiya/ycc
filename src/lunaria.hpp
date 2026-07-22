#ifndef LUNARIA_H
#define LUNARIA_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <deque>
#include <memory>
#include <cassert>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <stack>
#include <format>
#include <filesystem>
#include <bit>      // std::has_single_bit
#include <concepts> // std::integral
#include <array>

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
    STRUCT,
  };

  struct Type;
  struct Member{
    std::shared_ptr<Type> type;
    std::string name;
    int offset;
  };

  struct Type {
    TypeKind kind;
    std::shared_ptr<Type> base;
    int size;
    int align;
    int array_size;
    bool is_incomplete; //whether index is omitted
    std::list<std::shared_ptr<Member>> member; //struct
    
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
  std::shared_ptr<Type> struct_type();
  
  struct Initializer {
    int size;
    long val;

    //reference to other global variable
    std::string label;
    long addend;

    Initializer(int sz, long v)
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
    Var(const std::string& _name, const std::shared_ptr<Type>& _type, bool _isLocal)
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

  bool is_int32(int64_t v);
  std::shared_ptr<Type> get_larger_type(const std::shared_ptr<Type>& t1, const std::shared_ptr<Type>& t2);
} //namespace Lunaria

#include "slcc.hpp"

//---------------------------
//HIR
namespace Lunaria::LIR {
    struct BasicBlock;
  }

namespace Lunaria::HIR {
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
  HIR_MEMBER, //.
  HIR_CAST,
  HIR_NULL,
};
  
struct HirNode {
  HirKind kind;
  std::shared_ptr<HirNode> lhs;
  std::shared_ptr<HirNode> rhs;
  long long val;

  std::shared_ptr<Var> var;

  std::shared_ptr<HirNode> cond; //if,while,for
  std::shared_ptr<HirNode> then; //if,while,for
  std::shared_ptr<HirNode> els; //if
  std::shared_ptr<HirNode> init; //for
  std::shared_ptr<HirNode> inc; //for

  //break and continue
  std::shared_ptr<HirNode> target; 
  std::shared_ptr<LIR::BasicBlock> _break;
  std::shared_ptr<LIR::BasicBlock> _continue;

  std::vector<std::shared_ptr<HirNode>> cases;
  std::shared_ptr<HirNode> _default;
  std::shared_ptr<LIR::BasicBlock> _case_bb;

  std::list<std::shared_ptr<HirNode>> body;

  std::string funcName; //function name
  std::list<std::shared_ptr<HirNode>> args;

  std::shared_ptr<Type> type;

  std::shared_ptr<Member> member;
};

  struct Function {
    std::string name;
    std::list<std::shared_ptr<Var>> params;
    std::list<std::shared_ptr<HirNode>> body;
    //std::unordered_map<std::string, std::shared_ptr<Lunaria::Var>> localVars;
    std::unordered_set<std::shared_ptr<Var>, VarSharedPtrHash, VarSharedPtrEqual> localVars;
  };

  struct Program {
    std::list<std::unique_ptr<Function>> fns;
    //std::unordered_map<std::string, std::shared_ptr<Lunaria::Var>> globalVars;
    std::unordered_set<std::shared_ptr<Var>, VarSharedPtrHash, VarSharedPtrEqual> globalVars;
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
generateHirNode(const std::unique_ptr<Selene::Parser::Program>&);
  void add_type(std::shared_ptr<HirNode>& node);
} //namespace Lunaria::HIR
//---------------------------
//LIR
//namespace myLIR {
namespace Lunaria::LIR {
enum class LirKind {
  LIR_REG,
  LIR_MOV,
  LIR_IMM,
  LIR_ADD,
  LIR_SUB,
  LIR_MUL,
  LIR_MULHIGH,
  LIR_MAD,
  LIR_DIV,
  LIR_REM,
  LIR_LT, //<
  LIR_LE, //<=
  LIR_EQ, //==
  LIR_NE, //!=
  LIR_LVAR,
  LIR_LOAD,
  LIR_LOAD_SPILL,
  LIR_LOAD_STACK,
  LIR_LOAD_LABEL,
  LIR_STORE,
  LIR_STORE_SPILL,  
  LIR_STORE_ARG,
  LIR_STORE_STACK,
  LIR_STORE_LABEL,
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
  LIR_CAST,
  LIR_NULL,
};

  struct LirNode;
  using LirNodePtr = std::shared_ptr<LirNode>;
  using BasicBlockPtr = std::shared_ptr<BasicBlock>;
  
  struct BasicBlock {
    int label;
    std::list<LirNodePtr> insts;
    std::list<BasicBlockPtr> pred;
    std::list<BasicBlockPtr> succ;
    LirNodePtr param;
    
    bool is_start_node;
    bool is_end_node;
  };
  
  struct LirNode {
  public:
    LirKind opcode; //d = a <opcode> b
    LirNodePtr d; //destination operand
    LirNodePtr a; //left source operand
    LirNodePtr b; //right source operand
    int64_t imm;
    int scale;
    
    int vn; //virtual register number
    int rn; //real register number
    int def;
    int lastUse;
    bool spill;
    bool is_fixed_reg;
    int frn; //fixed register number
    
    std::shared_ptr<Var> lvar;
    std::string name; //for global variable
    
    BasicBlockPtr bb1;
    BasicBlockPtr bb2;
    LirNodePtr bbarg;
    
    std::string funcName;
    std::vector<LirNodePtr> args;
    
    int type_size;
    int type_base_size;
    
    LirNode(): opcode(LirKind::LIR_NULL), d(nullptr),
	       a(nullptr), b(nullptr), imm(-1), scale(0),
	       vn(-1), rn(-1), def(0), lastUse(0), spill(false),
	       is_fixed_reg(false), frn(-1),
	       lvar(nullptr), name(""),
	       bb1(nullptr), bb2(nullptr), bbarg(nullptr),
	       funcName(""), args({}),
	       type_size(0), type_base_size(0)
    {}
    std::vector<LirNodePtr> Defs() const;
    std::vector<LirNodePtr> Uses() const;
    /*
    const std::vector<LirNodePtr>& Defs() {calc_defs(); return _defs;};
    const std::vector<LirNodePtr>& Uses() {calc_uses(); return _uses;};
    std::vector<LirNodePtr>& get_mutable_defs() {calc_defs(); return _defs;};
    std::vector<LirNodePtr>& get_mutable_uses() {calc_uses(); return _uses;};
    */
    /*
  private:
    std::vector<LirNodePtr> _defs;
    std::vector<LirNodePtr> _uses;
    void calc_defs();
    void calc_uses();
    */
  };
  
  struct Function {
    std::string name;
    std::list<std::shared_ptr<Var>> params;
    std::unordered_set<std::shared_ptr<Var>, VarSharedPtrHash, VarSharedPtrEqual> localVars;
    int stackSize;
    std::list<BasicBlockPtr> bbs;
    BasicBlockPtr start_node;
    BasicBlockPtr end_node;
    int max_reg_pressure;

    std::list<BasicBlockPtr> get_topological_sort();
    std::list<BasicBlockPtr> get_reverse_topological_sort();

  private:
    std::list<BasicBlockPtr> topo_order;
    std::list<BasicBlockPtr> reverse_topo_order;
    void calc_topological_sort();
    void calc_reverse_topological_sort();
    void depth_first_search(const BasicBlockPtr& bb,
			    std::list<BasicBlockPtr>& order,
			    std::unordered_set<int>& mark);
    void depth_first_search_reverse(const BasicBlockPtr& bb,
				    std::list<BasicBlockPtr>& order,
				    std::unordered_set<int>& mark);
  };
  using FunctionPtr = std::shared_ptr<Function>;

  struct Program {
    std::list<FunctionPtr> fns;
    std::unordered_set<std::shared_ptr<Var>, VarSharedPtrHash, VarSharedPtrEqual> globalVars;
    std::unordered_map<std::string, int> funcname_to_reg_pressure;
  };

  LirNodePtr new_reg(const std::string& varName = "", int type_size = 0);
  std::unique_ptr<Program>
  generateLirNode(const std::unique_ptr<HIR::Program>&);
  std::string print_lir(const LirNodePtr& i, bool is_cfg_mode);
  void dumpLIR(const std::unique_ptr<Program>& prog, const std::string& filename);

  namespace Optimizer {
    void optimize(std::unique_ptr<Program>& prog,
		  const std::string& filename,
		  bool opt,
		  bool emit_cfg);
  } //namespace Optimizer
  
} //namespace Lunaria::LIR

//---------------------------
//Register Allocation
namespace Lunaria::RegAlloc {
  void preprocess_x86_64(std::unique_ptr<LIR::Program>& prog);
  void allocateRegister_x86_64(std::unique_ptr<LIR::Program>& prog);
}

//---------------------------
//Code Generation
namespace Lunaria::CodeGen {
  void gen_x86_64(const std::unique_ptr<LIR::Program>&, bool opt);
}


#endif
