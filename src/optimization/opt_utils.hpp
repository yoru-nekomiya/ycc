#ifndef OPT_UTILS_HPP
#define OPT_UTILS_HPP

#include "../ycc.hpp"

namespace myLIR::opt {
  bool optimize_bb(std::shared_ptr<BasicBlock>& bb);
  void constructCFGs(std::unique_ptr<Program>& prog);
  void printCFGs(std::unique_ptr<Program>& prog,
		 const std::string& filename);
  std::shared_ptr<LirNode> make_node(LirKind k);
  std::shared_ptr<LirNode> make_imm_node(int64_t imm);
} //namespace myLIR::opt

#endif
