#ifndef UTIL_HPP
#define UTIL_HPP

//#include "slcc.hpp"
#include "lunaria.hpp"

namespace Lunaria::LIR {
  bool is_imm(const LirNodePtr& lirNode);
  bool is_int32(const LirNodePtr& lirNode);
  bool is_imm_int32(const LirNodePtr& n);
} //namespace Lunaria::LIR

#endif

