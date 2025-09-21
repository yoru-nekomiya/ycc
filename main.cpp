#include "ycc.hpp"

std::queue<std::unique_ptr<Token>> tokens = {};

int main(int argc, char* argv[]){
  const std::string input = argv[1];
  tokenize(input);
  auto astNode = program();
  auto hirNode = generateHirNode(astNode);
  auto bbList = generateLirNode(hirNode);
  allocateRegister_x86_64(bbList);  
  gen_x86_64(bbList);
  return 0;
}
