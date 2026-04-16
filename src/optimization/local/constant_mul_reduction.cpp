#include "constant_mul_reduction.hpp"

namespace myLIR::opt {

  static void convert_mul_to_shift(std::shared_ptr<LirNode>& node,
				   int64_t c,
				   std::list<std::shared_ptr<myLIR::LirNode>>::iterator& iter,
				   std::shared_ptr<myLIR::BasicBlock>& bb){
    const int num_shift = get_log2(c);
    /*
    auto shift_node = std::make_shared<LirNode>();
    shift_node->opcode = LirKind::LIR_SHL;
    shift_node->a = node;      
    shift_node->b = make_imm_node(num_shift); 
    */
    auto shift_node = make_node(LirKind::LIR_SHL,
				nullptr,
				node,
				make_imm_node(num_shift));
    if(c >= 0){
      //d <- a * c
      //-->
      //d <- a << log2(c)      
      shift_node->d = (*iter)->d;      

      iter = bb->insts.erase(iter);
      iter = bb->insts.insert(iter, shift_node);
    } else {
      //d <- a * c
      //-->
      //d2 <- a << log2(c)
      //d <- 0 - d2
      shift_node->d = new_reg("");
      /*
      auto sub_node = std::make_shared<LirNode>();
      sub_node->opcode = LirKind::LIR_SUB;
      sub_node->d = (*iter)->d;
      sub_node->a = make_imm_node(0); 
      sub_node->b = shift_node->d;
      */
      auto sub_node = make_node(LirKind::LIR_SUB,
				(*iter)->d,
				make_imm_node(0),
				shift_node->d);
      iter = bb->insts.erase(iter);
      iter = bb->insts.insert(iter, sub_node);
      iter = bb->insts.insert(iter, shift_node);      
      iter++;
    }
  }
  
  static bool convert_optimized_mul(std::shared_ptr<LirNode>& n,
				    int64_t k,
				    std::list<std::shared_ptr<myLIR::LirNode>>::iterator& iter,
				    std::shared_ptr<myLIR::BasicBlock>& bb){
    //convert "n * k" to optimized instruction sequence using lea and shl
    assert(k != 1);
    assert(k != -1);
    assert(k != 0);
    assert(!is_abs_power_of_two(k));
    auto inst = *iter;

    const bool is_negative = (k < 0);
    const uint64_t abs_k = is_negative ? -k : k;

    //k = odd_k * 2^s
    const int s = std::countr_zero(abs_k);
    const uint64_t odd_k = abs_k >> s;

    std::shared_ptr<LirNode> mad_node = nullptr;
    std::shared_ptr<LirNode> shl_node = nullptr;
    std::shared_ptr<LirNode> add_node = nullptr;
    std::shared_ptr<LirNode> sub_node = nullptr;
    std::shared_ptr<LirNode> s_node = nullptr;
    std::shared_ptr<LirNode> neg_node = nullptr;
    std::shared_ptr<LirNode> mov_node = nullptr;
    int inst_count = 0;
    if(is_abs_power_of_two(odd_k - 1)){
      const int num = std::countr_zero(odd_k - 1);
      //use lea or shl+add
      //x = n * k --> x = mad(n, 2^num, n) n*2^num+n
      if(num <= 3){
	//lea (3,5,9)
	mad_node = make_node(LirKind::LIR_MAD,
			     inst->d,
			     n,
			     n);
	/*
	mad_node->d = inst->d;
	mad_node->a = n;
	mad_node->b = n;
	*/
	mad_node->scale = (1ULL << num);
	inst_count++;
      } else {
	//shl+add (17,33, ...)
	//x = n * k --> d = n << num; x = d + n;
	shl_node = make_node(LirKind::LIR_SHL,
			     new_reg(""),
			     n,
			     make_imm_node(num));
	/*
	shl_node->d = new_reg("");
	shl_node->a = n;
	shl_node->b = make_imm_node(num);
	*/
	
	add_node = make_node(LirKind::LIR_ADD,
			     inst->d,
			     shl_node->d,
			     n);
	/*
	add_node->d = inst->d;
	add_node->a = shl_node->d;
	add_node->b = n;
	*/
	inst_count += 2;
      }
    } else if(is_abs_power_of_two(odd_k + 1)){
      //shl+sub (7,15,31, ...)
      //x = n * k --> d = n << num; x = d - n;
      const int num = std::countr_zero(odd_k + 1);
      shl_node = make_node(LirKind::LIR_SHL,
			   new_reg(""),
			   n,
			   make_imm_node(num));
      /*
      shl_node->d = new_reg("");
      shl_node->a = n;
      shl_node->b = make_imm_node(num);
      */
      
      sub_node = make_node(LirKind::LIR_SUB,
			   inst->d,
			   shl_node->d,
			   n);
      /*
      sub_node->d = inst->d;
      sub_node->a = shl_node->d;
      sub_node->b = n;
      */
      inst_count += 2;
    } else {
      //mul
      return false;
    }

    if(s > 0){
      s_node = make_node(LirKind::LIR_SHL,
			 inst->d,
			 inst->d,
			 make_imm_node(s));
      /*
      s_node->d = inst->d;
      s_node->a = inst->d;
      s_node->b = make_imm_node(s);
      */
      inst_count++;
    }

    if(is_negative){
      neg_node = make_node(LirKind::LIR_SUB,
			   new_reg(""),
			   make_imm_node(0),
			   inst->d);
      /*
      neg_node->d = new_reg("");
      neg_node->a = make_imm_node(0);
      neg_node->b = inst->d;
      */
      inst_count++;

      mov_node = make_node(LirKind::LIR_MOV,
			   inst->d,
			   nullptr,
			   neg_node->d);
      /*
      mov_node->d = inst->d;
      mov_node->b = neg_node->d;
      */
    }

    if(inst_count > 3){
      return false;
    }

    int it_count = 0;
    iter = bb->insts.erase(iter);
    if(neg_node){
      iter = bb->insts.insert(iter, mov_node);
      it_count++;
    }
    if(neg_node){
      iter = bb->insts.insert(iter, neg_node);
      it_count++;
    }
    if(s_node){
      iter = bb->insts.insert(iter, s_node);
      it_count++;
    }
    if(sub_node){
      iter = bb->insts.insert(iter, sub_node);
      it_count++;
    }
    if(add_node){
      iter = bb->insts.insert(iter, add_node);
      it_count++;
    }
    if(shl_node){
      iter = bb->insts.insert(iter, shl_node);
      it_count++;
    }
    if(mad_node){
      iter = bb->insts.insert(iter, mad_node);
      it_count++;
    }
    std::advance(iter, it_count-1);
    return true;
  }
  
  bool reduce_mul(std::list<std::shared_ptr<myLIR::LirNode>>::iterator& iter,
			 std::shared_ptr<myLIR::BasicBlock>& bb){
    auto inst = *iter;
    bool changed = false;
    if(is_imm(inst->a) && !is_imm(inst->b)){
      if(inst->a->imm == 1){
	//d = 1 * b --> d = b
	auto mov_node = make_node(LirKind::LIR_MOV,
				  inst->d,
				  nullptr,
				  inst->b);
	/*
	mov_node->d = inst->d;
	mov_node->b = inst->b;
	*/
	iter = bb->insts.erase(iter);
	iter = bb->insts.insert(iter, mov_node);
	changed = true;
      }
      else if(inst->a->imm == -1){
	//d = -1 * b --> d = 0 - b
	auto sub_node = make_node(LirKind::LIR_SUB,
				  new_reg(""),
				  make_imm_node(0),
				  inst->b);
	/*
	sub_node->d = new_reg("");
	sub_node->a = make_imm_node(0);
	sub_node->b = inst->b;
	*/
	iter = bb->insts.erase(iter);
	iter = bb->insts.insert(iter, sub_node);
	++iter;

	auto mov_node = make_node(LirKind::LIR_MOV,
				  inst->d,
				  nullptr,
				  sub_node->d);
	/*
	mov_node->d = inst->d;
	mov_node->b = sub_node->d;
	*/
	iter = bb->insts.insert(iter, mov_node);	
	changed = true;
      }
      else if(inst->a->imm == 0){
	//d = 0 * b --> d = 0		
	auto imm_node = make_imm_node(0);
	imm_node->d = inst->d;
	iter = bb->insts.erase(iter);
	iter = bb->insts.insert(iter, imm_node);	
	changed = true;
      }
      else if(is_abs_power_of_two(inst->a->imm)){
	convert_mul_to_shift(inst->b, inst->a->imm, iter, bb);
	changed = true;
      }
      else {
	changed = convert_optimized_mul(inst->b, inst->a->imm, iter, bb);
      }
    }
    else if(!is_imm(inst->a) && is_imm(inst->b)){
      if(inst->b->imm == 1){
	//d = a * 1 --> d = a
	auto mov_node = make_node(LirKind::LIR_MOV,
				  inst->d,
				  nullptr,
				  inst->a);
	/*
	mov_node->d = inst->d;
	mov_node->b = inst->a;
	*/
	iter = bb->insts.erase(iter);
	iter = bb->insts.insert(iter, mov_node);
	changed = true;
      }
      else if(inst->b->imm == -1){
	//d = a * -1 --> d = 0 - a
	auto sub_node = make_node(LirKind::LIR_SUB,
				  new_reg(""),
				  make_imm_node(0),
				  inst->a);
	/*
	sub_node->d = new_reg("");
	sub_node->a = make_imm_node(0);
	sub_node->b = inst->a;
	*/
	iter = bb->insts.erase(iter);
	iter = bb->insts.insert(iter, sub_node);
	++iter;

	auto mov_node = make_node(LirKind::LIR_MOV,
				  inst->d,
				  nullptr,
				  sub_node->d);
	/*
	mov_node->d = inst->d;
	mov_node->b = sub_node->d;
	*/
	iter = bb->insts.insert(iter, mov_node);
	changed = true;
      }
      else if(inst->b->imm == 0){
	//d = a * 0 --> d = 0	
	auto imm_node = make_imm_node(0);
	imm_node->d = inst->d;
	iter = bb->insts.erase(iter);
	iter = bb->insts.insert(iter, imm_node);
	changed = true;
      }
      else if(is_abs_power_of_two(inst->b->imm)){
	convert_mul_to_shift(inst->a, inst->b->imm, iter, bb);
	changed = true;
      }
      else {
	changed = convert_optimized_mul(inst->a, inst->b->imm, iter, bb);
      }
    }
    return changed;
  }
  
}
