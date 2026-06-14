#include "graph_coloring_reg_alloc.hpp"

//physical regs (id:0 - 6): "rax", "rdi", "rsi", "rdx", "rcx", "r8", "r9"
//physical regs (id:7 - 13 ): "r10", "r11", "rbx", "r12", "r13", "r14", "r15"
//virtual regs (id: 14 - ): v0, v1, ...
static const std::string physical_regs[] = {"rax", "rdi", "rsi", "rdx", "rcx", "r8", "r9",
					    "r10", "r11", "rbx", "r12", "r13", "r14", "r15"};
static const int COLOR = std::size(physical_regs);

namespace myLIR {
  struct LirSharedPtrHash {      
   size_t operator()(const std::shared_ptr<LirNode>& p) const { 
     return std::hash<LirNode*>()(p.get());    
   }
  };

  struct BasicBlockSharedPtrHash {      
   size_t operator()(const std::shared_ptr<BasicBlock>& p) const { 
     return std::hash<BasicBlock*>()(p.get());    
   }
  };
}

namespace myRegAlloc {
  struct LiveRangeNode {
    int id;
    bool is_physical;
    int degree;
    //std::vector<int> adj_list;
    std::unordered_set<int> adj_list;
    int alias;
    bool is_move_related;
    bool is_removed;
    bool is_potential_spill;
    bool is_actual_spill;
    int color;
    bool on_stack;
  };

  struct InterferenceGraph {
    std::vector<LiveRangeNode> nodes;
    std::vector<std::vector<bool>> adj_matrix;
    std::vector<std::shared_ptr<myLIR::LirNode>> move_list;
    std::list<int> simplify_worklist;
    std::list<int> move_worklist;
    std::list<int> freeze_worklist;
    std::list<int> potential_spill_worklist;
    std::stack<int> select_stack;
  };

  static InterferenceGraph graph;
  static std::unordered_map<int, std::shared_ptr<myLIR::LirNode>> id_to_lirnode;
  static std::unordered_map<std::shared_ptr<myLIR::LirNode>, int> lirnode_to_id;

  using PredSet = std::unordered_set<int>;
  std::unordered_map<int, PredSet> bb_to_gen, bb_to_kill, bb_to_in, bb_to_out;

  static int get_alias(int id){
    if(graph.nodes[id].alias == id){
      return id;
    }
    return graph.nodes[id].alias = get_alias(graph.nodes[id].alias);
  }
  
  static std::vector<std::shared_ptr<myLIR::LirNode>>
  collect_reg(std::shared_ptr<myLIR::Function>& fn){
    std::vector<std::shared_ptr<myLIR::LirNode>> list_reg;
    std::unordered_set<std::shared_ptr<myLIR::LirNode>, myLIR::LirSharedPtrHash> inserted;
    for(auto& bb: fn->bbs){
      if(bb->param /*&& !bb->param->is_fixed_reg*/
	 && !inserted.contains(bb->param)){	
	list_reg.push_back(bb->param);
	inserted.insert(bb->param);
      }
      for(auto& lirNode: bb->insts){
	if(lirNode->d /*&& !lirNode->d->is_fixed_reg*/
	   && !inserted.contains(lirNode->d)){
	  list_reg.push_back(lirNode->d);
	  inserted.insert(lirNode->d);
	}
      }
    } //for bb
    return list_reg;
  }

  static void
  initialize_interference_graph(const std::vector<std::shared_ptr<myLIR::LirNode>>& list_reg){
    graph.nodes.clear();
    graph.adj_matrix.clear();
    graph.move_list.clear();
    graph.simplify_worklist.clear();
    graph.move_worklist.clear();
    graph.freeze_worklist.clear();
    graph.potential_spill_worklist.clear();
    id_to_lirnode.clear();
    lirnode_to_id.clear();
    bb_to_gen.clear(); bb_to_kill.clear(); bb_to_in.clear(); bb_to_out.clear();
    
    for(int i = 0; i < std::size(physical_regs); i++){
      LiveRangeNode n;
      n.id = i;
      n.is_physical = true;
      n.degree = 0;
      n.adj_list.clear();
      n.alias = i;
      n.is_move_related = false;
      n.is_removed = false;
      n.is_potential_spill = false;
      n.is_actual_spill = false;
      n.color = i;
      n.on_stack = true;
      graph.nodes.push_back(n);
    }
    
    int i = std::size(physical_regs);
    for(int j = 0; j < list_reg.size(); j++){
      LiveRangeNode n;
      n.id = list_reg[j]->is_fixed_reg ? list_reg[j]->frn : i;
      n.is_physical = list_reg[j]->is_fixed_reg;
      n.degree = 0;
      n.adj_list.clear();
      n.alias = n.id;
      n.is_move_related = false;
      n.is_removed = false;
      n.is_potential_spill = false;
      n.is_actual_spill = false;
      n.color = list_reg[j]->is_fixed_reg ? list_reg[j]->frn : -1;
      n.on_stack = false;
      graph.nodes.push_back(n);
      if(!list_reg[j]->is_fixed_reg){
	auto [iter, inserted] = id_to_lirnode.insert_or_assign(n.id, list_reg[j]);
	if(!inserted){
	  std::cerr << "update existing value to " << n.id << std::endl;
	}
      }
      lirnode_to_id.insert(std::make_pair(list_reg[j], n.id));
      i++;
    }

    const int size = std::size(physical_regs) + list_reg.size() + 1;
    graph.adj_matrix.assign(size, std::vector<bool>(size, false));
  }

  static void compute_local_predicate(std::shared_ptr<myLIR::Function>& fn){
    const auto rev_topo = fn->get_reverse_topological_sort();
    for(const auto& bb: rev_topo){
      PredSet gen, kill;
      for(auto iter = bb->insts.rbegin(); iter != bb->insts.rend(); iter++){
	auto& inst = *iter;
	if(inst->opcode == myLIR::LirKind::LIR_IMM
	   || inst->opcode == myLIR::LirKind::LIR_LABEL_ADDR
	   || inst->opcode == myLIR::LirKind::LIR_LOAD_STACK
	   || inst->opcode == myLIR::LirKind::LIR_LVAR
	   || inst->opcode == myLIR::LirKind::LIR_LOAD_SPILL){
	  gen.erase(lirnode_to_id.at(inst->d));
	  kill.insert(lirnode_to_id.at(inst->d));
	  continue;
	}
	
	if(myLIR::opt::is_binary_opcode(inst->opcode)){
	  gen.erase(lirnode_to_id.at(inst->d));
	  kill.insert(lirnode_to_id.at(inst->d));	  

	  if(inst->opcode == myLIR::LirKind::LIR_MULHIGH
	     || inst->opcode == myLIR::LirKind::LIR_DIV
	     || inst->opcode == myLIR::LirKind::LIR_REM){
	    gen.erase(0);   //rax
	    kill.insert(0); //rax
	    gen.erase(3);   //rdx
	    kill.insert(3); //rdx
	  }

	  if(inst->opcode == myLIR::LirKind::LIR_SHL
	     || inst->opcode == myLIR::LirKind::LIR_SHR
	     || inst->opcode == myLIR::LirKind::LIR_SAR){
	    if(!myLIR::is_imm_int32(inst->b)){
	      gen.erase(4);   //rcx
	      kill.insert(4); //rcx
	    }
	  }

	  if(inst->opcode == myLIR::LirKind::LIR_PTR_DIFF){
	    const int s = inst->type_base_size;
	    if(s != 1 && s != 2 && s != 4 && s != 8){
	      gen.erase(0);   //rax
	      kill.insert(0); //rax
	      gen.erase(3);   //rdx
	      kill.insert(3); //rdx
	    }
	  }
	  if(!myLIR::is_imm_int32(inst->a)) gen.insert(lirnode_to_id.at(inst->a));
	  if(!myLIR::is_imm_int32(inst->b)) gen.insert(lirnode_to_id.at(inst->b));
	  continue;
	} //if is_binary_opcode
	
	if(inst->opcode == myLIR::LirKind::LIR_MOV
	   || inst->opcode == myLIR::LirKind::LIR_LOAD
	   || inst->opcode == myLIR::LirKind::LIR_CAST){
	  gen.erase(lirnode_to_id.at(inst->d));
	  kill.insert(lirnode_to_id.at(inst->d));	  	  
	  if(!myLIR::is_imm_int32(inst->b)) gen.insert(lirnode_to_id.at(inst->b));	  
	  continue;
	}
		
	if(inst->opcode == myLIR::LirKind::LIR_BR
	   || inst->opcode == myLIR::LirKind::LIR_STORE_STACK){
	  if(!myLIR::is_imm_int32(inst->b)) gen.insert(lirnode_to_id.at(inst->b));
	  continue;
	}

	if(inst->opcode == myLIR::LirKind::LIR_JMP){
	  if(inst->bbarg)
	    if(!myLIR::is_imm_int32(inst->bbarg)) gen.insert(lirnode_to_id.at(inst->bbarg));	  
	  continue;
	}
	
	if(inst->opcode == myLIR::LirKind::LIR_STORE){
	  gen.insert(lirnode_to_id.at(inst->a));
	  if(!myLIR::is_imm_int32(inst->b)) gen.insert(lirnode_to_id.at(inst->b));
	  continue;
	}
	
	if(inst->opcode == myLIR::LirKind::LIR_STORE_SPILL){
	  gen.insert(lirnode_to_id.at(inst->a));
	  continue;
	}

	if(inst->opcode == myLIR::LirKind::LIR_STORE_ARG){
	  const auto arg_num = inst->imm + 1;
	  gen.insert(arg_num);
	  continue;
	}
	
	if(inst->opcode == myLIR::LirKind::LIR_RETURN){
	  if(inst->a != nullptr){	    
	    if(!myLIR::is_imm_int32(inst->a)) gen.insert(lirnode_to_id.at(inst->a));	    
	  }
	  continue;
	}
	
	if(inst->opcode == myLIR::LirKind::LIR_FUNCALL){	  
	  gen.erase(lirnode_to_id.at(inst->d));
	  kill.insert(lirnode_to_id.at(inst->d));

	  //rax, rdi - r9
	  for(int i = 0; i <= 6; i++){
	    gen.erase(i);
	    kill.insert(i);
	  }
	  
	  for(int i = 0; i < inst->args.size(); i++){
	    if(!myLIR::is_imm_int32(inst->args[i])){
	      gen.insert(lirnode_to_id.at(inst->args[i]));
	    }
	  }
	  continue;
	}
	
      } //for iter
      bb_to_gen.insert(std::make_pair(bb->label, gen));
      bb_to_kill.insert(std::make_pair(bb->label, kill));
      bb_to_in.insert(std::make_pair(bb->label, PredSet()));
      bb_to_out.insert(std::make_pair(bb->label, PredSet()));  
    } //for bb
  }

  static void compute_dataflow_equation(std::shared_ptr<myLIR::Function>& fn){
    auto worklist = fn->get_reverse_topological_sort();
    std::unordered_set<std::shared_ptr<myLIR::BasicBlock>, myLIR::BasicBlockSharedPtrHash> workset(worklist.begin(), worklist.end());
    while(!worklist.empty()){
      auto bb = worklist.front();
      worklist.pop_front();

      auto in_old = bb_to_in[bb->label];
      auto gen = bb_to_gen[bb->label];
      auto kill = bb_to_kill[bb->label];

      //Compute OUT set
      PredSet out;
      if(!bb->is_end_node){
	for(const auto& s: bb->succ){
	  const auto in_succ = bb_to_in[s->label];	  
	  for(const auto& item: in_succ) out.insert(item);
	} //for s
	bb_to_out[bb->label] = out;
      } //if

      //Compute IN set
      auto in = out;
      for(const auto& item: kill) in.erase(item);
      for(const auto& item: gen) in.insert(item);
      bb_to_in[bb->label] = in;

      //Compare IN set with the previous IN set
      bool changed = false;      
      if(in_old != in) changed = true;
      if(changed){
	for(const auto& p: bb->pred){
	  if(!workset.contains(p)){
	    worklist.push_back(p);
	    workset.insert(p);
	  }
	}
      } //if
    } //while
  }

  static void add_edge(int u, int v){
    if(u == v) return;
    if(graph.adj_matrix[u][v]) return;
    if(graph.nodes[u].is_physical && graph.nodes[v].is_physical) return;
    
    //Update matrix
    graph.adj_matrix[u][v] = true;
    graph.adj_matrix[v][u] = true;

    //Update adj_list of each node
    //graph.nodes[u].adj_list.push_back(v);
    //graph.nodes[v].adj_list.push_back(u);
    graph.nodes[u].adj_list.insert(v);
    graph.nodes[v].adj_list.insert(u);

    graph.nodes[u].degree++;
    graph.nodes[v].degree++;
  }

  static void build_interference_graph(std::shared_ptr<myLIR::Function>& fn){
    for(const auto& bb: fn->bbs){
      auto live = bb_to_out[bb->label];
      for(auto iter = bb->insts.rbegin(); iter != bb->insts.rend(); iter++){
	auto& inst = *iter;
	if(inst->opcode == myLIR::LirKind::LIR_MOV){

	  if(!myLIR::is_imm_int32(inst->b)){
	    graph.move_list.push_back(inst);
	    graph.nodes[lirnode_to_id.at(inst->d)].is_move_related = true;
	    graph.nodes[lirnode_to_id.at(inst->b)].is_move_related = true;
	  }
	  for(auto live_reg: live){
	    if(!myLIR::is_imm_int32(inst->b))
	      if(live_reg == lirnode_to_id.at(inst->b))
		continue;	    	    
	    add_edge(lirnode_to_id.at(inst->d), live_reg);
	  }
	  
	  live.erase(lirnode_to_id.at(inst->d));
	  if(!myLIR::is_imm_int32(inst->b)) live.insert(lirnode_to_id.at(inst->b));
	  continue;
	} //LIR_MOV

	if(inst->opcode == myLIR::LirKind::LIR_IMM
	   || inst->opcode == myLIR::LirKind::LIR_LABEL_ADDR
	   || inst->opcode == myLIR::LirKind::LIR_LOAD_STACK
	   || inst->opcode == myLIR::LirKind::LIR_LVAR
	   || inst->opcode == myLIR::LirKind::LIR_LOAD_SPILL){
	  //Add the edge from Defs of this instruction to live registers immediately after this instruction
	  for(auto r: live){
	    add_edge(lirnode_to_id.at(inst->d), r);
	  }
	  live.erase(lirnode_to_id.at(inst->d));
	  continue;
	} //LIR_IMM, LIR_LABEL_ADDR

	if(myLIR::opt::is_binary_opcode(inst->opcode)){
	  for(auto r: live)
	    add_edge(lirnode_to_id.at(inst->d), r);
	  if (!myLIR::is_imm_int32(inst->a) && !myLIR::is_imm_int32(inst->b))
	    add_edge(lirnode_to_id.at(inst->a), lirnode_to_id.at(inst->b));
	  
	  if(inst->opcode == myLIR::LirKind::LIR_MULHIGH
	     || inst->opcode == myLIR::LirKind::LIR_DIV
	     || inst->opcode == myLIR::LirKind::LIR_REM){
	    for(auto r: live){
	      add_edge(0, r); //rax
	      add_edge(3, r); //rdx
	    }
	    if(!myLIR::is_imm_int32(inst->a)){
	      add_edge(0, lirnode_to_id.at(inst->a));
	      add_edge(3, lirnode_to_id.at(inst->a));
	    }
	    if(!myLIR::is_imm_int32(inst->b)){
	      add_edge(0, lirnode_to_id.at(inst->b));
	      add_edge(3, lirnode_to_id.at(inst->b));
	    }
	    live.erase(0);
	    live.erase(3);
	  }
	  if(inst->opcode == myLIR::LirKind::LIR_SHL
	     || inst->opcode == myLIR::LirKind::LIR_SHR
	     || inst->opcode == myLIR::LirKind::LIR_SAR){
	    for(auto r: live)
	      add_edge(4, r); //rcx
	    if(!myLIR::is_imm_int32(inst->a))
	      add_edge(4, lirnode_to_id.at(inst->a)); //rcx
	    if(!myLIR::is_imm_int32(inst->b))
	      add_edge(4, lirnode_to_id.at(inst->b)); //rcx
	      
	    live.erase(4);
	  }
	  if(inst->opcode == myLIR::LirKind::LIR_PTR_DIFF){
	    const int s = inst->type_base_size;
	    if(s != 1 && s != 2 && s != 4 && s != 8){
	      for(auto r: live){
		add_edge(0, r); //rax
		add_edge(3, r); //rdx
	      }
	      if(!myLIR::is_imm_int32(inst->a)){
		add_edge(0, lirnode_to_id.at(inst->a));
		add_edge(3, lirnode_to_id.at(inst->a));
	      }
	      if(!myLIR::is_imm_int32(inst->b)){
		add_edge(0, lirnode_to_id.at(inst->b));
		add_edge(3, lirnode_to_id.at(inst->b));
	      }
	      live.erase(0);
	      live.erase(3);
	    }
	  }
	  
	  live.erase(lirnode_to_id.at(inst->d));
	  if(!myLIR::is_imm_int32(inst->a)) live.insert(lirnode_to_id.at(inst->a));
	  if(!myLIR::is_imm_int32(inst->b)) live.insert(lirnode_to_id.at(inst->b));
	  continue;
	} //binary_opcode

	if(inst->opcode == myLIR::LirKind::LIR_LOAD
	   || inst->opcode == myLIR::LirKind::LIR_CAST){
	  for(auto r: live)
	    add_edge(lirnode_to_id.at(inst->d), r);
	  
	  live.erase(lirnode_to_id.at(inst->d));
	  if(!myLIR::is_imm_int32(inst->b)) live.insert(lirnode_to_id.at(inst->b));
	  continue;
	} //LIR_LOAD, LIR_CAST
	
	if(inst->opcode == myLIR::LirKind::LIR_BR
	   || inst->opcode == myLIR::LirKind::LIR_STORE_STACK){
	  if(!myLIR::is_imm_int32(inst->b)) live.insert(lirnode_to_id.at(inst->b));
	  continue;
	} //LIR_BR, LIR_STORE_STACK

	if(inst->opcode == myLIR::LirKind::LIR_JMP){
	  if(inst->bbarg){
	    for(auto r: live)
	      add_edge(lirnode_to_id.at(inst->bb1->param), r);

	    live.erase(lirnode_to_id.at(inst->bb1->param));
	    if(!myLIR::is_imm_int32(inst->bbarg)) live.insert(lirnode_to_id.at(inst->bbarg));
	  }
	  continue;
	} //LIR_JMP

	if(inst->opcode == myLIR::LirKind::LIR_STORE){
	  live.insert(lirnode_to_id.at(inst->a));
	  if(!myLIR::is_imm_int32(inst->b)) live.insert(lirnode_to_id.at(inst->b));
	  continue;
	} //LIR_STORE

	if(inst->opcode == myLIR::LirKind::LIR_STORE_SPILL){
	  live.insert(lirnode_to_id.at(inst->a));
	  continue;
	} //LIR_STORE_SPILL

	if(inst->opcode == myLIR::LirKind::LIR_STORE_ARG){
	  const auto arg_num = inst->imm + 1;
	  live.insert(arg_num);
	  continue;
	} //LIR_STORE_ARG

	if(inst->opcode == myLIR::LirKind::LIR_RETURN){
	  if(inst->a != nullptr){	    
	    if(!myLIR::is_imm_int32(inst->a)) live.insert(lirnode_to_id.at(inst->a));	    
	  }
	  continue;
	} //LIR_RETURN

	if(inst->opcode == myLIR::LirKind::LIR_FUNCALL){
	  for(auto r: live){
	    add_edge(lirnode_to_id.at(inst->d), r);
	    for(int i = 0; i <= 6; i++)
	      add_edge(i, r); //rax, rdi - r9
	  }
	  for(int i = 0; i < inst->args.size(); i++){
	    if(!myLIR::is_imm_int32(inst->args[i])){
	      for(int p = 0; p <= 6; p++){
		add_edge(p, lirnode_to_id.at(inst->args[i]));
	      }
	    }
	  }
	  
	  live.erase(lirnode_to_id.at(inst->d));	  
	  for(int i = 0; i <= 6; i++){
	    live.erase(i); //rax, rdi - r9
	  }
	  
	  for(int i = 0; i < inst->args.size(); i++){
	    if(!myLIR::is_imm_int32(inst->args[i])){
	      live.insert(lirnode_to_id.at(inst->args[i]));
	    }
	  }
	  continue;
	} //LIR_FUNCALL
	
      } //for iter
    } //for bb
  }
  
  static void liveness_analysis(std::shared_ptr<myLIR::Function>& fn){
    compute_local_predicate(fn);
    compute_dataflow_equation(fn);
  }

  static void initialize_worklists(){
    /*
    graph.simplify_worklist.clear();
    graph.move_worklist.clear();
    graph.freeze_worklist.clear();
    graph.potential_spill_worklist.clear();
    */
    for(int i = COLOR; i < graph.nodes.size(); i++){
      auto& node = graph.nodes[i];
      if(node.is_physical) continue;

      if(node.degree >= COLOR){
	graph.potential_spill_worklist.push_back(i);
      }
      else {	
	if(node.is_move_related){
	  //graph.freeze_worklist.push_back(i);
	  graph.move_worklist.push_back(i);
	} else {
	  graph.simplify_worklist.push_back(i);
	}	
	//graph.simplify_worklist.push_back(i);
      }
    } //for i
  }

  static void simplify(){
    while(!graph.simplify_worklist.empty()){
      const int id = graph.simplify_worklist.front();
      graph.simplify_worklist.pop_front();

      const int alias = get_alias(id);
      if(graph.nodes[alias].is_physical || graph.nodes[alias].on_stack) continue;
      
      graph.select_stack.push(/*id*/alias);
      graph.nodes[/*id*/alias].is_removed = true;
      graph.nodes[alias].on_stack = true;
      for(auto j: graph.nodes[/*id*/alias].adj_list){
	if(!graph.nodes[j].is_removed){
	  graph.nodes[j].degree--;
	}
      }
    } //while
  }
  
  static void merge(int u, int v) {
    //merge v to u (u is a representative node)
    graph.nodes[v].alias = u;
    
    for (auto t : graph.nodes[v].adj_list) {
      add_edge(u, t);

      //delete the edge t --> v
      graph.nodes[t].adj_list.erase(v);
      graph.nodes[t].degree--;
    }
    
    //reset the degree of v
    graph.nodes[v].degree = 0;
    graph.nodes[v].adj_list.clear();
  }

  static bool is_Briggs_safe(int u, int v){
    int k_or_more_count = 0;
    
    std::unordered_set<int> combined_adj = graph.nodes[u].adj_list;
    combined_adj.insert(graph.nodes[v].adj_list.begin(), graph.nodes[v].adj_list.end());
    
    for(int t : combined_adj){
      if(t == u || t == v) continue;      
      if(graph.nodes[t].degree >= COLOR){
	k_or_more_count++;
      }
    }    
    return k_or_more_count < COLOR;
  }
  
  static void coalesce(){
    for(const auto& inst: graph.move_list){
      const int d = inst->d->is_fixed_reg ? inst->d->frn : lirnode_to_id.at(inst->d);
      const int b = inst->b->is_fixed_reg ? inst->b->frn : lirnode_to_id.at(inst->b);           
      int u = get_alias(d);
      int v = get_alias(b);

      if(graph.nodes[v].is_physical){
	std::swap(u, v);
      }

      if(u == v){
	//u and v is already the same color
	continue;
      }

      if(graph.nodes[v].is_physical || graph.adj_matrix[u][v]){
	//both u and v are physical registers
	//or u and v are interfering with each other
	continue;
      }

      bool is_safe = false;
      if(graph.nodes[u].is_physical){
	//In this case, u is a physical register and v is a virtual register
	//George: coalesce node n1 and n2 to n12
	//        iff all neighbors of n1 either already interfere with n2 or are of insignificant degree.
	is_safe = true;
	for(auto t: graph.nodes[v].adj_list){
	  if(!(graph.adj_matrix[t][u] || graph.nodes[t].degree < COLOR)){
	    is_safe = false;
	    break;
	  }
	}
	//if(is_safe) std::cerr << "---George" << std::endl;
      }

      if(!graph.nodes[u].is_physical){
	//In this case, both u and v are not physical registers
	//Briggs: coalesce nodes n1 and n2 to n12
	//        iff n12 has less than K neighbors of significant degree (i..e. of a degree greater or equal to K)
	/*
	if(graph.nodes[u].degree + graph.nodes[v].degree < COLOR){
	  //This is a simple version
	  is_safe = true;
	  std::cerr << "---Briggs" << std::endl;
	}
	*/
	if(is_Briggs_safe(u, v)){
	  is_safe = true;
	  //std::cerr << "---Briggs" << std::endl;
	}
      }      
      if(is_safe) merge(u, v);
      //if(!is_safe) std::cerr << "---Not merged" << std::endl;
    } //for inst

    while(!graph.move_worklist.empty()){
      const int id = graph.move_worklist.front();
      graph.move_worklist.pop_front();

      const int alias = get_alias(id);
      if(graph.nodes[alias].is_physical) continue;
      
      graph.select_stack.push(/*id*/alias);
      graph.nodes[/*id*/alias].is_removed = true;
      graph.nodes[alias].on_stack = true;
      for(auto j: graph.nodes[/*id*/alias].adj_list){
	if(!graph.nodes[j].is_removed){
	  graph.nodes[j].degree--;
	}
      }
    } //while
  }
  
  static void select_spill(){
    //TODO: cost calculation is needed.
    while(!graph.potential_spill_worklist.empty()){
      const int id = graph.potential_spill_worklist.front();
      graph.potential_spill_worklist.pop_front();

      const int alias = get_alias(id);
      if(graph.nodes[alias].is_physical) continue;
      
      graph.select_stack.push(/*id*/alias);
      graph.nodes[/*id*/alias].is_removed = true;
      graph.nodes[alias].on_stack = true;
      
      if(graph.nodes[/*id*/alias].degree >= COLOR){
	graph.nodes[/*id*/alias].is_potential_spill = true;
      }
      for(auto j: graph.nodes[/*id*/alias].adj_list){
	if(!graph.nodes[j].is_removed){
	  graph.nodes[j].degree--;
	}
      }
    } //while
  }

  static void select_color(){
    while(!graph.select_stack.empty()){
      const int id = graph.select_stack.top();
      graph.select_stack.pop();

      //const int alias = get_alias(id);
      graph.nodes[/*alias*/id].on_stack = false;
      if(graph.nodes[/*alias*/id].color != -1) continue;
	
      std::unordered_set<int> already_colored;
      graph.nodes[id/*alias*/].is_removed = false;
      for(auto r: graph.nodes[id/*alias*/].adj_list){
	if(!graph.nodes[r].is_removed && graph.nodes[r].color != -1){
	  already_colored.insert(graph.nodes[r].color);
	}
      }
      for(int i = 0; i < COLOR; i++){
	if(!already_colored.contains(i)){
	  graph.nodes[id/*alias*/].color = i;
	  break;
	}
      }
      if(graph.nodes[id/*alias*/].color == -1){
	graph.nodes[id/*alias*/].is_actual_spill = true;
	assert(false && "spilled!!!");
      }
      
    } //while
  }

  static int assign_reg_number(){
    int max_reg_pressure = INT_MIN;    
    for(const auto& n: graph.nodes){
      if(!n.is_physical){
	if(n.id == get_alias(n.id)){
	  auto lirNode = id_to_lirnode.at(n.id);
	  lirNode->rn = n.color;
	  max_reg_pressure = std::max(max_reg_pressure, n.color);
	  //std::cerr << lirNode->vn << " is colored to " << n.color << std::endl;
	}
      }
    } //for n

    for(const auto& n: graph.nodes){
      if(!n.is_physical
	 && n.id != get_alias(n.id)){
	const int alias = get_alias(n.id);
	auto lirNode = id_to_lirnode.at(n.id);
	if(alias < COLOR){
	  lirNode->rn = alias;
	} else {
	  auto lirNode_alias = id_to_lirnode.at(alias);
	  lirNode->rn = lirNode_alias->rn;
	}
      }
    } //for n
    return max_reg_pressure;
  }
  
  void graph_coloring_register_allocation_x86_64(std::unique_ptr<myLIR::Program>& prog){
    preprocess_x86_64(prog);
    for(auto& fn: prog->fns){
      auto list_reg = collect_reg(fn);
      initialize_interference_graph(list_reg);
      liveness_analysis(fn);
      build_interference_graph(fn);
      initialize_worklists();
      simplify();
      coalesce();
      select_spill();
      select_color();
      const int max_reg_pressure = assign_reg_number();
      fn->max_reg_pressure = max_reg_pressure-7 < 0 ? -1 : max_reg_pressure-7;
      prog->funcname_to_reg_pressure.insert(std::make_pair(fn->name, fn->max_reg_pressure));
    }
  }
} //namespace myRegAlloc
