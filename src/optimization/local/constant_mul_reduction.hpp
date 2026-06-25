#ifndef CONSTANT_MUL_REDUCTION_HPP
#define CONSTANT_MUL_REDUCTION_HPP

#include "../../ycc.hpp"
#include "../../util.hpp"
#include "../opt_utils.hpp"

namespace Lunaria::LIR::Optimizer {
  bool reduce_mul(std::list<std::shared_ptr<LirNode>>::iterator& iter,
		  std::shared_ptr<BasicBlock>& bb);
}

#endif
