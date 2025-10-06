#include "ycc.hpp"

int main(int argc, char* argv[]){
  const std::string input = argv[1];
  myTokenizer::tokenize(input);
  auto prog = myParser::program();
  auto progHir = myHIR::generateHirNode(prog);
  auto progLir = myLIR::generateLirNode(progHir);
  allocateRegister_x86_64(progLir);  
  gen_x86_64(progLir);
  return 0;
}
