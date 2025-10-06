#include "ycc.hpp"

namespace myHIR {
static std::unique_ptr<HirNode>
new_node(HirKind kind){
  auto node = std::make_unique<HirNode>();
  node->kind = kind;
  return node;
}

static std::unique_ptr<HirNode>
new_binary(HirKind kind,
	   std::unique_ptr<HirNode>& lhs,
	   std::unique_ptr<HirNode>& rhs){
  auto hirNode = new_node(kind);
  hirNode->lhs = std::move(lhs);
  hirNode->rhs = std::move(rhs);
  return hirNode;
}

static std::unique_ptr<HirNode>
new_num(const std::unique_ptr<myParser::AstNode>& astNode){
  auto hirNode = new_node(HirKind::HIR_IMM);
  hirNode->val = astNode->val;
  return hirNode;
}

static std::unique_ptr<HirNode>
new_lvar(const std::unique_ptr<myParser::AstNode>& astNode){
  auto hirNode = new_node(HirKind::HIR_LVAR);
  hirNode->lvar = std::move(astNode->lvar);
  return hirNode;
}

std::unique_ptr<HirNode>
program(const std::unique_ptr<myParser::AstNode>& astNode){
  if(astNode->kind == myParser::AstKind::AST_NUM){
    return new_num(astNode);
  } else if(astNode->kind == myParser::AstKind::AST_LVAR){
    return new_lvar(astNode);
  } else if(astNode->kind == myParser::AstKind::AST_RETURN){
    auto lhs = program(astNode->lhs);
    auto hirNode = new_node(HirKind::HIR_RETURN);
    hirNode->lhs = std::move(lhs);
    return hirNode;
  }
  else if(astNode->kind == myParser::AstKind::AST_DEREF){
    auto lhs = program(astNode->lhs);
    auto hirNode = new_node(HirKind::HIR_DEREF);
    hirNode->lhs = std::move(lhs);
    return hirNode;
  }
  else if(astNode->kind == myParser::AstKind::AST_ADDR){
    auto lhs = program(astNode->lhs);
    auto hirNode = new_node(HirKind::HIR_ADDR);
    hirNode->lhs = std::move(lhs);
    return hirNode;
  }
  else if(astNode->kind == myParser::AstKind::AST_BLOCK){
    auto hirNode = new_node(HirKind::HIR_BLOCK);
    for(const auto& n: astNode->body){
      hirNode->body.push_back(program(n));
    }
    return hirNode;
  } else if(astNode->kind == myParser::AstKind::AST_FUNCALL){
    auto hirNode = new_node(HirKind::HIR_FUNCALL);
    hirNode->funcName = astNode->funcName;
    for(const auto& an: astNode->args){
      hirNode->args.push_back(program(an));
    }
    return hirNode;
  } else if(astNode->kind == myParser::AstKind::AST_IF){
    auto hirNode = new_node(HirKind::HIR_IF);
    auto cond = program(astNode->cond);
    auto then = program(astNode->then);
    std::unique_ptr<HirNode> els = nullptr;
    if(astNode->els){
      els = program(astNode->els);
    }
    hirNode->cond = std::move(cond);
    hirNode->then = std::move(then);
    hirNode->els = std::move(els);
    return hirNode;
  } else if(astNode->kind == myParser::AstKind::AST_WHILE){
    auto hirNode = new_node(HirKind::HIR_WHILE);
    auto cond = program(astNode->cond);
    auto then = program(astNode->then);
    hirNode->cond = std::move(cond);
    hirNode->then = std::move(then);
    return hirNode;
  } else if(astNode->kind == myParser::AstKind::AST_FOR){
    auto hirNode = new_node(HirKind::HIR_FOR);
    std::unique_ptr<HirNode> init = nullptr;
    std::unique_ptr<HirNode> cond = nullptr;
    std::unique_ptr<HirNode> inc = nullptr;
    if(astNode->init){
      init = program(astNode->init);
    }
    if(astNode->cond){
      cond = program(astNode->cond);
    }
    if(astNode->inc){
      inc = program(astNode->inc);
    }

    auto then = program(astNode->then);
    hirNode->init = std::move(init);
    hirNode->cond = std::move(cond);
    hirNode->inc = std::move(inc);
    hirNode->then = std::move(then);
    return hirNode;
  } else {
    auto lhs = program(astNode->lhs);
    auto rhs = program(astNode->rhs);
    switch(astNode->kind){
    case myParser::AstKind::AST_ADD:
      return new_binary(HirKind::HIR_ADD, lhs, rhs);
    case myParser::AstKind::AST_SUB:
      return new_binary(HirKind::HIR_SUB, lhs, rhs);
    case myParser::AstKind::AST_MUL:
      return new_binary(HirKind::HIR_MUL, lhs, rhs);
    case myParser::AstKind::AST_DIV:
      return new_binary(HirKind::HIR_DIV, lhs, rhs);
    case myParser::AstKind::AST_LT:
      return new_binary(HirKind::HIR_LT, lhs, rhs);
    case myParser::AstKind::AST_LE:
      return new_binary(HirKind::HIR_LE, lhs, rhs);
    case myParser::AstKind::AST_EQ:
      return new_binary(HirKind::HIR_EQ, lhs, rhs);
    case myParser::AstKind::AST_NE:
      return new_binary(HirKind::HIR_NE, lhs, rhs);
    case myParser::AstKind::AST_ASSIGN:
      return new_binary(HirKind::HIR_ASSIGN, lhs, rhs);
    } //switch
  }
  return nullptr;
}

std::unique_ptr<Program>
generateHirNode(const std::unique_ptr<myParser::Program>& prog){
  auto progHir = std::make_unique<Program>();
  for(auto& fn: prog->fns){
    auto fnHir = std::make_unique<Function>();
    fnHir->name = fn->name;
    fnHir->params = fn->params;
    fnHir->localVars = fn->localVars;
    for(auto& astNode: fn->body){
      fnHir->body.push_back(program(astNode));
    }
    progHir->fns.push_back(std::move(fnHir));
  }
  return progHir;
}
} //namespace myHIR
