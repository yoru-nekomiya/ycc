#ifndef OPT_UTILS_HPP
#define OPT_UTILS_HPP

#include "../ycc.hpp"

namespace myLIR::opt {
  bool optimize_bb(std::shared_ptr<BasicBlock>& bb);
  bool optimize_fn(std::shared_ptr<Function>& fn);
  void constructCFGs(std::unique_ptr<Program>& prog);
  void printCFGs(std::unique_ptr<Program>& prog,
		 const std::string& filename);
  std::shared_ptr<LirNode> make_node(LirKind k);
  std::shared_ptr<LirNode> make_imm_node(int64_t imm);
  bool is_binary_opcode(LirKind k);

  template <std::integral T>
  bool is_abs_power_of_two(T n){
    // 0 は2の冪乗ではない
    if(n == 0) return false;
    
    // 負の最小値 (INT_MINなど) の絶対値は、正の最大値 (INT_MAX) を超えるため
    // 安全に扱うために符号なし型 (std::make_unsigned_t<T>) にキャストする
    using U = std::make_unsigned_t<T>;
    
    U abs_n;
    if(n < 0){
      // n が負の場合、2の補数表現での絶対値を計算
      // (単に -n とすると INT_MIN でオーバーフローする可能性があるためキャストが先)
      abs_n = static_cast<U>(0) - static_cast<U>(n);
    } else {
      abs_n = static_cast<U>(n);
    }
    
    // C++20: 立っているビットが1つだけなら true
    return std::has_single_bit(abs_n);
  }
  
  template <std::integral T>
  int get_log2(T n){
    assert(n != 0);
    using U = std::make_unsigned_t<T>;
    return std::countr_zero(static_cast<U>(n));
  }
  
} //namespace myLIR::opt

#endif
