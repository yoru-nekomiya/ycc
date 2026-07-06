#ifndef CONSTANT_DIV_REDUCTION_HPP
#define CONSTANT_DIV_REDUCTION_HPP

//#include "../../slcc.hpp"
#include "../../lunaria.hpp"
#include "../../util.hpp"
#include "../opt_utils.hpp"

namespace Lunaria::LIR::Optimizer {
  
bool reduce_div_and_rem(std::list<LirNodePtr>::iterator& iter,
			BasicBlockPtr& bb);

}
#endif
