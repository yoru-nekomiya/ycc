#include "constant_div_reduction.hpp"

namespace myLIR::opt {
  struct MagicS64 {
    int64_t M;      // magic number
    int32_t s;      // additional shift
    bool use_add;   // Add-Shift is necessary or not
  };
  
  static void convert_div_and_rem_to_shift(std::shared_ptr<LirNode>& x,
					   int64_t c,
					   std::list<std::shared_ptr<myLIR::LirNode>>::iterator& iter,
					   std::shared_ptr<myLIR::BasicBlock>& bb){
    //d = x / c
    //-->
    //k = log2(|c|)
    //temp = x >> 63 (SAR)
    //bias = temp >>> (64-k) (SHR)
    //x2 = x + bias
    //d = x2 >> k (SAR)

    //d = x % c
    //-->
    //k = log2(|c|)
    //temp = x >> 63 (SAR)
    //bias = temp >>> (64-k) (SHR)
    //x2 = x + bias
    //mask = |c| - 1
    //x3 = x2 & mask
    //d = x3 - bias
    const LirKind kind = (*iter)->opcode;
    const int k = get_log2(c);
    const int mask = std::abs(c) - 1;
    
    auto sar_node_1 = make_node(LirKind::LIR_SAR);
    sar_node_1->d = new_reg("");
    sar_node_1->a = x;
    sar_node_1->b = make_imm_node(63);

    auto shr_node = make_node(LirKind::LIR_SHR);
    shr_node->d = new_reg("");
    shr_node->a = sar_node_1->d;
    shr_node->b = make_imm_node(64-k);

    auto add_node = make_node(LirKind::LIR_ADD);
    add_node->d = new_reg("");
    add_node->a = x;
    add_node->b = shr_node->d;

    if(kind == LirKind::LIR_DIV){
      auto sar_node_2 = make_node(LirKind::LIR_SAR);
      sar_node_2->a = add_node->d;
      sar_node_2->b = make_imm_node(k);
      
      if(c >= 0){
	sar_node_2->d = (*iter)->d;
	
	iter = bb->insts.erase(iter);
	iter = bb->insts.insert(iter, sar_node_2);
	iter = bb->insts.insert(iter, add_node);
	iter = bb->insts.insert(iter, shr_node);
	iter = bb->insts.insert(iter, sar_node_1);
	std::advance(iter, 3);
      } else {
	//d2 = x2 >> k (SAR)
	//d = 0-d2 
	sar_node_2->d = new_reg("");
	
	auto sub_node = make_node(LirKind::LIR_SUB);
	sub_node->d = (*iter)->d;
	sub_node->a = make_imm_node(0);
	sub_node->b = sar_node_2->d;
	
	iter = bb->insts.erase(iter);
	iter = bb->insts.insert(iter, sub_node);
	iter = bb->insts.insert(iter, sar_node_2);
	iter = bb->insts.insert(iter, add_node);
	iter = bb->insts.insert(iter, shr_node);
	iter = bb->insts.insert(iter, sar_node_1);
	std::advance(iter, 4);
      }
    }    
    else if(kind == LirKind::LIR_REM){
      auto and_node = make_node(LirKind::LIR_BITAND);
      and_node->d = new_reg("");
      and_node->a = add_node->d;
      and_node->b = make_imm_node(mask);

      auto sub_node = make_node(LirKind::LIR_SUB);
      sub_node->d = (*iter)->d;
      sub_node->a = and_node->d;
      sub_node->b = shr_node->d;

      iter = bb->insts.erase(iter);
      iter = bb->insts.insert(iter, sub_node);
      iter = bb->insts.insert(iter, and_node);
      iter = bb->insts.insert(iter, add_node);
      iter = bb->insts.insert(iter, shr_node);
      iter = bb->insts.insert(iter, sar_node_1);
      std::advance(iter, 4);
    }    
  }

  MagicS64 compute_magic_s64(int64_t d) {
    using uint128 = __uint128_t;
    
    // 1. Obtaining the absolute value (calculated using unsigned 128-bit)
    uint64_t ad = (d < 0) ? -static_cast<uint64_t>(d) : static_cast<uint64_t>(d);
    
    // 2. Setting boundary values ​​(based on 2^63)
    uint128 t = ((uint128)1 << 63) + (static_cast<uint64_t>(d) >> 63);
    uint128 anc = t - 1 - (t % ad);
    int32_t p = 63;
    
    uint128 q1 = ((uint128)1 << 63) / anc;
    uint128 r1 = ((uint128)1 << 63) % anc;
    uint128 q2 = ((uint128)1 << 63) / ad;
    uint128 r2 = ((uint128)1 << 63) % ad;
    uint128 delta;

    // 3. Loop until the required accuracy is achieved
    do {
        p++;
        q1 = 2 * q1;
        r1 = 2 * r1;
        if (r1 >= anc) { q1++; r1 -= anc; }
        q2 = 2 * q2;
        r2 = 2 * r2;
        if (r2 >= ad) { q2++; r2 -= ad; }
        delta = ad - r2;
    } while (q1 < delta || (q1 == delta && r1 == 0));

    // 4. Confirmation of results
    uint128 M_u = q2 + 1;
    bool use_add = (M_u >= ((uint128)1 << 63)); // If it exceeds 63 bits, use Add-Shift
    
    int64_t magic_M = static_cast<int64_t>(M_u);
    //if (d < 0) magic_M = -magic_M;

    return {magic_M, p - 64, use_add};
  }

  static void reduce_div_using_magic(std::list<std::shared_ptr<myLIR::LirNode>>::iterator& iter,
				     std::shared_ptr<myLIR::BasicBlock>& bb){
    //inst is a division "x = n / d" where d is constant
    auto inst = *iter;

    //Compute magic number M
    auto magic = compute_magic_s64(inst->b->imm);

    //q = MulHigh(n, M)
    auto q = new_reg("");
    auto mulhigh_node = make_node(LirKind::LIR_MULHIGH);
    mulhigh_node->d = q;
    mulhigh_node->a = inst->a;
    mulhigh_node->b = make_imm_node(magic.M);
    iter = bb->insts.erase(iter);
    iter = bb->insts.insert(iter, mulhigh_node);
    ++iter;

    //If necessary, q = Add(q, n)
    //std::shared_ptr<LirNode> add_node;
    if(magic.use_add){
      auto add_node = make_node(LirKind::LIR_ADD);
      add_node->d = q; 
      add_node->a = q; 
      add_node->b = inst->a;
      iter = bb->insts.insert(iter, add_node);
      ++iter;
    }

    //q = Sar(q, s)
    auto sar_node = make_node(LirKind::LIR_SAR);
    sar_node->d = q; 
    sar_node->a = q; 
    sar_node->b = make_imm_node(magic.s);
    iter = bb->insts.insert(iter, sar_node);
    ++iter;

    //If n is signed value,
    //sign = Shr(n, 63)
    //x = Add(q, sign)
    auto sign = new_reg("");
    auto shr_node = make_node(LirKind::LIR_SHR);
    shr_node->d = sign; 
    shr_node->a = inst->a;
    shr_node->b = make_imm_node(63);
    iter = bb->insts.insert(iter, shr_node);
    ++iter;

    auto add_sign = make_node(LirKind::LIR_ADD);
    add_sign->d = new_reg("");
    add_sign->a = q; 
    add_sign->b = sign; 
    iter = bb->insts.insert(iter, add_sign);
    ++iter;
    
    if(inst->b->imm < 0){
      //if d < 0, then reverse the sign
      auto sub_node = make_node(LirKind::LIR_SUB);
      sub_node->d = inst->d;
      sub_node->a = make_imm_node(0);
      sub_node->b = add_sign->d;
      iter = bb->insts.insert(iter, sub_node);
    } else {
      auto mov_node = make_node(LirKind::LIR_MOV);
      mov_node->d = inst->d;
      mov_node->b = add_sign->d;
      iter = bb->insts.insert(iter, mov_node);
    }
  }

  static void reduce_rem_to_div(std::list<std::shared_ptr<myLIR::LirNode>>::iterator& iter,
				std::shared_ptr<myLIR::BasicBlock>& bb){
    //inst is a remainder "n % d" where d is constant
    auto inst = *iter;

    //Convert "x = n % d" to "x = n - (n/d) * d"
    auto div_node = make_node(LirKind::LIR_DIV);
    div_node->d = new_reg("");
    div_node->a = inst->a;
    div_node->b = inst->b;
    iter = bb->insts.erase(iter);
    iter = bb->insts.insert(iter, div_node);
    ++iter;
    
    auto mul_node = make_node(LirKind::LIR_MUL);
    mul_node->d = new_reg("");
    mul_node->a = div_node->d;
    mul_node->b = inst->b;
    iter = bb->insts.insert(iter, mul_node);
    ++iter;

    auto sub_node = make_node(LirKind::LIR_SUB);
    sub_node->d = inst->d;
    sub_node->a = inst->a;
    sub_node->b = mul_node->d;
    iter = bb->insts.insert(iter, sub_node);
  }
  
  bool reduce_div_and_rem(std::list<std::shared_ptr<myLIR::LirNode>>::iterator& iter,
			  std::shared_ptr<myLIR::BasicBlock>& bb){
    auto inst = *iter;
    bool changed = false;
    if(!is_imm(inst->a) && is_imm(inst->b)){
      if(inst->b->imm == 1 || inst->b->imm == -1){
	if(inst->opcode == LirKind::LIR_DIV){
	  //d = x / 1  --> d = x
	  //d = x / -1 --> d = 0-x
	  if(inst->b->imm == 1){
	    auto mov_node = make_node(LirKind::LIR_MOV);
	    mov_node->d = inst->d;
	    mov_node->b = inst->a;
	    iter = bb->insts.erase(iter);
	    iter = bb->insts.insert(iter, mov_node);
	  } else if(inst->b->imm == -1){
	    auto sub_node = make_node(LirKind::LIR_SUB);
	    sub_node->d = new_reg("");
	    sub_node->a = make_imm_node(0);
	    sub_node->b = inst->a;
	    iter = bb->insts.erase(iter);
	    iter = bb->insts.insert(iter, sub_node);
	    ++iter;
	    
	    auto mov_node = make_node(LirKind::LIR_MOV);
	    mov_node->d = inst->d;
	    mov_node->b = sub_node->d;
	    iter = bb->insts.insert(iter, mov_node);
	    
	  }
	} else if(inst->opcode == LirKind::LIR_REM){
	  //d = x % 1   --> d = 0
	  //d = x % -1  --> d = 0
	  auto imm_node = make_imm_node(0);
	  imm_node->d = inst->d;
	  iter = bb->insts.erase(iter);
	  iter = bb->insts.insert(iter, imm_node);
	}
	changed = true;
      } 
      else if(is_abs_power_of_two(inst->b->imm)){
	convert_div_and_rem_to_shift(inst->a, inst->b->imm, iter, bb);
	changed = true;
      }
      else {
	if(inst->opcode == LirKind::LIR_DIV){
	  reduce_div_using_magic(iter, bb);
	} else if(inst->opcode == LirKind::LIR_REM){
	  reduce_rem_to_div(iter, bb);
	}
	changed = true;
      }
    }
    return changed;
  }
  
}
