#include "ycc.hpp"

namespace Lunaria {
  std::shared_ptr<Type> int_type = std::make_shared<Type>(TypeKind::INT);

  std::shared_ptr<Type> pointer_to(const std::shared_ptr<Type>& base){
    auto type = std::make_shared<Type>(TypeKind::PTR, base);
    return type;
  }
  
} //namespace Lunaria
