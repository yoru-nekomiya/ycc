#ifndef UTIL_HPP
#define UTIL_HPP

#include "ycc.hpp"

namespace myLIR {
  bool is_imm(const std::shared_ptr<LirNode>& lirNode);
  bool is_int32(const std::shared_ptr<myLIR::LirNode>& lirNode);
  
} //namespace myLIR

#endif

