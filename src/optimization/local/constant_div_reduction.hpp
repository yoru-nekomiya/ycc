#ifndef CONSTANT_DIV_REDUCTION_HPP
#define CONSTANT_DIV_REDUCTION_HPP

#include "../../ycc.hpp"
#include "../../util.hpp"
#include "../opt_utils.hpp"

namespace Lunaria::LIR::Optimizer {
  
bool reduce_div_and_rem(std::list<std::shared_ptr<LirNode>>::iterator& iter,
			std::shared_ptr<BasicBlock>& bb);

}
#endif
