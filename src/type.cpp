#include "ycc.hpp"

namespace Lunaria {
  std::shared_ptr<Type> int_type = std::make_shared<Type>(TypeKind::INT, 4, 4); //kind, size, align
  std::shared_ptr<Type> char_type = std::make_shared<Type>(TypeKind::CHAR, 1, 1);
  std::shared_ptr<Type> short_type = std::make_shared<Type>(TypeKind::SHORT, 2, 2);
  std::shared_ptr<Type> long_type = std::make_shared<Type>(TypeKind::LONG, 8, 8);
  std::shared_ptr<Type> void_type = std::make_shared<Type>(TypeKind::VOID, 1, 1);
  
  bool is_integer(const std::shared_ptr<Type>& type){
    const auto k = type->kind;
    return k == TypeKind::INT
      || k == TypeKind::CHAR
      || k == TypeKind::SHORT
      || k == TypeKind::LONG
      ;
  }

  std::shared_ptr<Type> new_type(TypeKind kind, int size, int align){
    auto type = std::make_shared<Type>(kind, size, align);
    return type;
  }

  std::shared_ptr<Type> pointer_to(const std::shared_ptr<Type>& base){
    auto type = std::make_shared<Type>(TypeKind::PTR, base, 8, 8);
    return type;
  }

  int align_to(int n, int align){
    return (n+align-1) & ~(align-1);
  }

  std::shared_ptr<Type> array_of(const std::shared_ptr<Type>& base, int size){
    auto type = std::make_shared<Type>(TypeKind::ARRAY,
					base,
					base->size * size,
					base->align);
    type->array_size = size;
    return type;
  } 
} //namespace Lunaria

namespace myParser {
  void add_type(std::unique_ptr<AstNode>& node) {
    if (!node || node->type){
      return;
    }
    
    add_type(node->lhs);
    add_type(node->rhs);
    add_type(node->cond);
    add_type(node->then);
    add_type(node->els);
    add_type(node->init);
    add_type(node->inc);
    
    for(auto& n: node->body){
      add_type(n);
    }
    for(auto& n: node->args){
      add_type(n);
    }
    
    switch (node->kind) {
    case AstKind::AST_ADD:
    case AstKind::AST_SUB:
    case AstKind::AST_PTR_DIFF:
    case AstKind::AST_MUL:
    case AstKind::AST_DIV:
    case AstKind::AST_REM:
    case AstKind::AST_EQ:
    case AstKind::AST_NE:
    case AstKind::AST_LT:
    case AstKind::AST_LE:
    case AstKind::AST_LOGOR:
    case AstKind::AST_LOGAND:
    case AstKind::AST_NOT:
    case AstKind::AST_FUNCALL:
    case AstKind::AST_NUM:
    case AstKind::AST_BITOR:
    case AstKind::AST_BITXOR:
    case AstKind::AST_BITAND:
      node->type = Lunaria::int_type;
      return;
    case AstKind::AST_PTR_ADD:
    case AstKind::AST_PTR_SUB:
    case AstKind::AST_ASSIGN:
    case AstKind::AST_PRE_INC:
    case AstKind::AST_PRE_DEC:
    case AstKind::AST_POST_INC:
    case AstKind::AST_POST_DEC:
    case AstKind::AST_SHL:
    case AstKind::AST_SHR:
    case AstKind::AST_SAR:
    case AstKind::AST_BITNOT:
    case AstKind::AST_ADD_ASSIGN:
    case AstKind::AST_SUB_ASSIGN:
    case AstKind::AST_MUL_ASSIGN:
    case AstKind::AST_DIV_ASSIGN:
      node->type = node->lhs->type;
      return;
    case AstKind::AST_VAR:
      node->type = node->var->type;
      return;
    case AstKind::AST_ADDR: //address &
      //node->type = pointer_to(node->lhs->type);
      if(node->lhs->type->kind == Lunaria::TypeKind::ARRAY){
	node->type = pointer_to(node->lhs->type->base);
      } else {
	node->type = pointer_to(node->lhs->type);
      }
      return;
    case AstKind::AST_DEREF: //dereferrence *
      if(!node->lhs->type->base){
	std::cerr << "invalid pointer dereference" << std::endl;
	exit(1);
      } //if      
      node->type = node->lhs->type->base;
      if(node->type->kind == Lunaria::TypeKind::VOID){
	std::cerr << "dereference void pointer\n";
	exit(1);
      }
      return;
    case AstKind::AST_SUBSCRIPTED: //a[i]
      if(!node->lhs->type->base){
	std::cerr << "invalid array reference" << std::endl;
	exit(1);
      } //if
      node->type = node->lhs->type->base;      
      return;    
    } //switch()
  } //add_type()
  
} //namespace myParser

namespace myHIR {
  void add_type(std::shared_ptr<HirNode>& node){
    if (!node || node->type){
      return;
    }
    
    add_type(node->lhs);
    add_type(node->rhs);
    add_type(node->cond);
    add_type(node->then);
    add_type(node->els);
    add_type(node->init);
    add_type(node->inc);
    
    for(auto& n: node->body){
      add_type(n);
    }
    for(auto& n: node->args){
      add_type(n);
    }
    
    switch (node->kind) {
    case HirKind::HIR_ADD:
    case HirKind::HIR_SUB:
    case HirKind::HIR_PTR_DIFF:
    case HirKind::HIR_MUL:
    case HirKind::HIR_DIV:
    case HirKind::HIR_REM:
    case HirKind::HIR_EQ:
    case HirKind::HIR_NE:
    case HirKind::HIR_LT:
    case HirKind::HIR_LE:
    case HirKind::HIR_LOGOR:
    case HirKind::HIR_LOGAND:
    case HirKind::HIR_NOT:
    case HirKind::HIR_FUNCALL:
    case HirKind::HIR_IMM:
    case HirKind::HIR_BITOR:
    case HirKind::HIR_BITXOR:
    case HirKind::HIR_BITAND:
      node->type = Lunaria::int_type;
      return;
    case HirKind::HIR_PTR_ADD:
    case HirKind::HIR_PTR_SUB:
    case HirKind::HIR_ASSIGN:
    case HirKind::HIR_PRE_INC:
    case HirKind::HIR_PRE_DEC:
    case HirKind::HIR_POST_INC:
    case HirKind::HIR_POST_DEC:
    case HirKind::HIR_SHL:
    case HirKind::HIR_SHR:
    case HirKind::HIR_SAR:
    case HirKind::HIR_BITNOT:
    case HirKind::HIR_ADD_ASSIGN:
    case HirKind::HIR_SUB_ASSIGN:
    case HirKind::HIR_MUL_ASSIGN:
    case HirKind::HIR_DIV_ASSIGN:
      node->type = node->lhs->type;
      return;
    case HirKind::HIR_VAR:
      node->type = node->var->type;
      return;
    case HirKind::HIR_ADDR: //address &
      //node->type = pointer_to(node->lhs->type);
      if(node->lhs->type->kind == Lunaria::TypeKind::ARRAY){
	node->type = pointer_to(node->lhs->type->base);
      } else {
	node->type = pointer_to(node->lhs->type);
      }
      return;
    case HirKind::HIR_DEREF: //dereferrence *
      if(!node->lhs->type->base){
	std::cerr << "invalid pointer dereference" << std::endl;
	exit(1);
      } //if      
      node->type = node->lhs->type->base;
      if(node->type->kind == Lunaria::TypeKind::VOID){
	std::cerr << "dereference void pointer\n";
	exit(1);
      }
      return;
    case HirKind::HIR_SUBSCRIPTED: //a[i]
      if(!node->lhs->type->base){
	std::cerr << "invalid array reference" << std::endl;
	exit(1);
      } //if
      node->type = node->lhs->type->base;      
      return;    
    } //switch()
  } //add_type()
  
} //namespace myHIR
