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
} //namespace myLIR::opt

#endif
