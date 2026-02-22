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
      s->label = 0;
      s->is_start_node = true;
      s->is_end_node = false;

      auto e = std::make_shared<BasicBlock>();
      e->label = 0;
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
  
} //namespace myLIR::opt
