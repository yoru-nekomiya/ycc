#include "util.hpp"

namespace myLIR {
  bool is_imm(const std::shared_ptr<myLIR::LirNode>& lirNode){
    if(lirNode->opcode == myLIR::LirKind::LIR_IMM){
      return true;
    }
    return false;
  }

  bool is_int32(const std::shared_ptr<myLIR::LirNode>& lirNode){
    if(lirNode->imm >= (int64_t)INT32_MIN && lirNode->imm <= (int64_t)INT32_MAX){
      return true;
    }
    return false;
  }
} //namespace myLIR
