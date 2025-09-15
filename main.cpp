#include "ycc.hpp"

std::queue<std::unique_ptr<Token>> tokens = {};

int main(int argc, char* argv[]){
  const std::string input = argv[1];
  tokenize(input);
  auto astNode = program();
  auto hirNode = generateHirNode(astNode);
  auto lirList = generateLirNode(hirNode);
  allocateRegister_x86_64(lirList);

  
  std::cout << ".intel_syntax noprefix" << std::endl
	    << ".global main" << std::endl
	    << "main:" << std::endl;
  
  gen_x86_64(lirList);
  
  auto& t = lirList.back();
  const int d = t->d->rn;
  std::cout << "  mov rax, " << regs[d] << std::endl;
  std::cout << "  ret" << std::endl;
  return 0;
}
