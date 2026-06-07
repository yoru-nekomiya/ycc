#ifndef UTIL_HPP
#define UTIL_HPP

#include "ycc.hpp"

namespace myLIR {
  bool is_imm(const std::shared_ptr<LirNode>& lirNode);
  bool is_int32(const std::shared_ptr<LirNode>& lirNode);
  bool is_imm_int32(const std::shared_ptr<LirNode>& n);
} //namespace myLIR

#endif

