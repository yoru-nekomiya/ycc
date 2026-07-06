#ifndef CONSTANT_MUL_REDUCTION_HPP
#define CONSTANT_MUL_REDUCTION_HPP

//#include "../../slcc.hpp"
#include "../../lunaria.hpp"
#include "../../util.hpp"
#include "../opt_utils.hpp"

namespace Lunaria::LIR::Optimizer {
  bool reduce_mul(std::list<LirNodePtr>::iterator& iter,
		  BasicBlockPtr& bb);
}

#endif
