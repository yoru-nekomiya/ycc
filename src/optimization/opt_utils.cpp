#include "opt_utils.hpp"

namespace myLIR::opt {
  static void add_edges(std::shared_ptr<BasicBlock>& bb){
    if(bb->insts.empty()) return;
    if(!bb->succ.empty()) return;

    auto& last = bb->insts.back();
    if(last->bb1){
      bb->succ.push_back(last->bb1);
      last->bb1->pred.push_back(bb);
      add_edges(last->bb1);
    }
    if(last->bb2){
      bb->succ.push_back(last->bb2);
      last->bb2->pred.push_back(bb);
      add_edges(last->bb2);
    }
    return;
  }
  
  void constructCFGs(std::unique_ptr<Program>& prog){
    //This function constructs a Control Flow Graph (CFG) of "fn" in "prog".
    //While constructing it,
    //this function adds the unique start node "s" and end node "e".
    //Every node of "fn" lies on a path from "s" to "e".
    //"s" has no predecessors, and "e" has no successors.
    
    for(auto& fn: prog->fns){
      auto s = std::make_shared<BasicBlock>();
      s->label = -1;
      s->is_start_node = true;
      s->is_end_node = false;

      auto e = std::make_shared<BasicBlock>();
      e->label = -2;
      e->is_start_node = false;
      e->is_end_node = true;

      fn->start_node = s;
      fn->end_node = e;

      //start_node --> the first basic block of fn
      auto& first = fn->bbs.front();
      s->succ.push_back(first);
      first->pred.push_back(s);

      //add edges of the body of fn
      add_edges(first);

      //the nodes that have no successor --> end_node
      for(auto& bb: fn->bbs){
	if(bb->succ.empty()){
	  bb->succ.push_back(e);
	  e->pred.push_back(bb);
	}
      } //for bb
    } //for fn
  }

  static std::string
  get_dot_path(const std::string& input_path,
	       const std::string& funcname){
    std::filesystem::path p(input_path);
    std::filesystem::path dir = p.parent_path();
    if(dir.empty()){
      dir = ".";
    }

    std::string filename = std::format("{}_{}.dot", p.stem().string(), funcname);
    return (dir / filename).string();
  }

  static std::string escape_for_dot(const std::string& s) {
    std::string out;
    for(char c : s){
      if(c == '<' || c == '>' || c == '|' || c == '{' || c == '}'){
	out += '\\'; 
      }
      out += c;
    }
    return out;
  }

  void printCFGs(std::unique_ptr<Program>& prog,
		 const std::string& filename){
    for(const auto& fn: prog->fns){      
      const std::string dotfile = get_dot_path(filename, fn->name);
      std::ofstream f;
      f.open(dotfile);
      if(!f.is_open()){
	std::cerr << "cannot open lir file\n";
	exit(1);
      }

      f << std::format("digraph \"CFG for {} function\" {{\n", fn->name);
      f << "node [fontname=\"Courier New\", fontsize=10];\n";
      f << "\tNode_START [shape=record, color=\"#3d50c3ff\", style=filled, fillcolor=\"#e36c5570\",label=\"{start}\"];\n";
      f << "\tNode_END [shape=record, color=\"#3d50c3ff\", style=filled, fillcolor=\"#e36c5570\",label=\"{end}\"];\n";

      //edges for Node_START -> succ of Node_START
      for(const auto& s: fn->start_node->succ){
	f << std::format("\tNode_START -> Node_BB{};\n", s->label);
      }
      
      for(const auto& bb: fn->bbs){
	f << std::format("\tNode_BB{} [shape=record, color=\"#3d50c3ff\", style=filled, fillcolor=\"#e36c5570\",label=\"{{BB_{}:\\l", bb->label, bb->label);
	
	for(const auto& inst: bb->insts){
	  f << escape_for_dot(myLIR::print_lir(inst));
	  f << "\\l";
	} //for inst
	f << "}\"];\n";
	
	for(const auto& s: bb->succ){
	  if(s->is_end_node){
	    f << std::format("\tNode_BB{} -> Node_END;\n", bb->label);
	  } else {
	    f << std::format("\tNode_BB{} -> Node_BB{};\n", bb->label, s->label);	
	  }
	} //for s
      } //for bb
      f << "}";
      f.close();
    } //for fn
  }

  std::shared_ptr<LirNode>
  make_node(LirKind k,
	    const std::shared_ptr<LirNode>& d,
	    const std::shared_ptr<LirNode>& a,
	    const std::shared_ptr<LirNode>& b){
    auto n = std::make_shared<LirNode>();
    n->opcode = k;
    n->d = d;
    n->a = a;
    n->b = b;
    return n;
  }

  std::shared_ptr<LirNode> make_imm_node(int64_t imm){
    auto n = std::make_shared<LirNode>();
    n->opcode = LirKind::LIR_IMM;
    n->imm = imm;
    return n;
  }

  bool is_binary_opcode(LirKind k){
    return k == LirKind::LIR_ADD
      || k == LirKind::LIR_SUB
      || k == LirKind::LIR_MUL
      || k == LirKind::LIR_MULHIGH
      || k == LirKind::LIR_MAD
      || k == LirKind::LIR_DIV
      || k == LirKind::LIR_REM
      || k == LirKind::LIR_EQ
      || k == LirKind::LIR_NE
      || k == LirKind::LIR_LT
      || k == LirKind::LIR_LE
      || k == LirKind::LIR_PTR_ADD
      || k == LirKind::LIR_PTR_SUB
      || k == LirKind::LIR_PTR_DIFF
      || k == LirKind::LIR_SHL
      || k == LirKind::LIR_SHR
      || k == LirKind::LIR_SAR
      || k == LirKind::LIR_BITOR
      || k == LirKind::LIR_BITAND
      || k == LirKind::LIR_BITXOR;
  }
  
} //namespace myLIR::opt

namespace myLIR {
  void Function::depth_first_search(const std::shared_ptr<BasicBlock>& bb,
				    std::list<std::shared_ptr<BasicBlock>>& order,
				    std::unordered_set<int>& mark){
    if(!mark.contains(bb->label)){
      mark.insert(bb->label);    
      for(const auto& s: bb->succ){
	depth_first_search(s, order, mark);
      } //for    
      order.push_front(bb);
    } //if
  }
  
  void Function::calc_topological_sort(){
    //This function calculates topological sort order of basic blocks on an acyclic graph
    //or quasi-topological sort order of basic blocks on a cyclic graph
    this->topo_order.clear();
    std::unordered_set<int> mark = {};
    depth_first_search(this->start_node, this->topo_order, mark);  
    return; 
  }

  std::list<std::shared_ptr<BasicBlock>> Function::get_topological_sort(){
    //TODO: If a CFG has not been changed since it was previously calculated, this function should return the previous value without recalculating.
    calc_topological_sort();
    return this->topo_order;
  }

  void Function::depth_first_search_reverse(const std::shared_ptr<BasicBlock>& bb,
					    std::list<std::shared_ptr<BasicBlock>>& order,
					    std::unordered_set<int>& mark){
    if(!mark.contains(bb->label)){
      mark.insert(bb->label);
      for(const auto& p: bb->pred){
	depth_first_search_reverse(p, order, mark);
      } //for
      order.push_front(bb);
    } //if
  }
  
  void Function::calc_reverse_topological_sort(){
    //This function reverse-topological sort order of basic blocks on an acyclic graph
    //or quasi-reverse-topological sort order of basic blocks on a cyclic graph
    this->reverse_topo_order.clear();
    std::unordered_set<int> mark = {};
    depth_first_search_reverse(this->end_node, this->reverse_topo_order, mark);
    return;
  }

  std::list<std::shared_ptr<BasicBlock>> Function::get_reverse_topological_sort(){
    //TODO: If a CFG has not been changed since it was previously calculated, this function should return the previous value without recalculating.
    calc_reverse_topological_sort();
    return this->reverse_topo_order;
  }
  
} //namespace myLIR
