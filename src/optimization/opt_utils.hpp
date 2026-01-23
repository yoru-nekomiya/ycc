#ifndef OPT_UTILS_HPP
#define OPT_UTILS_HPP

#include "../ycc.hpp"
#include <memory>
#include <format>
#include <string_view>

namespace myLIR::opt {
  void constructCFGs(std::unique_ptr<Program>& prog);
  void printCFGs(std::unique_ptr<Program>& prog,
		 const std::string& filename);
  
} //namespace myLIR::opt

#endif
