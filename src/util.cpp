#include "util.hpp"

namespace Lunaria::LIR {
  bool is_imm(const LirNodePtr& lirNode){
    if(lirNode->opcode == LirKind::LIR_IMM){
      return true;
    }
    return false;
  }

  bool is_int32(const LirNodePtr& lirNode){
    if(lirNode->imm >= (int64_t)INT32_MIN && lirNode->imm <= (int64_t)INT32_MAX){
      return true;
    }
    return false;
  }

  bool is_imm_int32(const LirNodePtr& n){
    return is_imm(n) && is_int32(n);
  }

} //namespace Lunaria::LIR
