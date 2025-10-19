#include "ycc.hpp"

namespace Lunaria {
  std::shared_ptr<Type> int_type = std::make_shared<Type>(TypeKind::INT, 4, 4); //kind, size, align

  bool is_integer(const std::shared_ptr<Type>& type){
    return type->kind == TypeKind::INT;
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
    case AstKind::AST_EQ:
    case AstKind::AST_NE:
    case AstKind::AST_LT:
    case AstKind::AST_LE:
    case AstKind::AST_FUNCALL:
    case AstKind::AST_NUM:
      node->type = Lunaria::int_type;
      return;
    case AstKind::AST_PTR_ADD:
    case AstKind::AST_PTR_SUB:
    case AstKind::AST_ASSIGN:
      node->type = node->lhs->type;
      return;
    case AstKind::AST_LVAR:
      node->type = node->lvar->type;
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
