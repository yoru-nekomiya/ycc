#include "ycc.hpp"

static std::string readFileToString(const std::string& filename){
  std::ifstream ifs(filename);
  if(!ifs){
    throw std::runtime_error("Failed to open file: " + filename);
  }
  std::stringstream buf;
  buf << ifs.rdbuf();
  return buf.str();
}

static std::string
replace_file_extension(const std::string& input_path,
		       const std::string& ext){
  std::filesystem::path p(input_path);
  p.replace_extension(std::format(".{}", ext));
  return p.string();
}

int main(int argc, char* argv[]){
  if(argc < 2){
    std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
    return 1;
  }
  try{
    const std::string filename = argv[1];
    const std::string input = readFileToString(filename);
    myTokenizer::tokenize(input);
    auto prog = myParser::program();
    auto progHir = myHIR::generateHirNode(prog);
    
    auto progLir = myLIR::generateLirNode(progHir);
    dumpLIR(progLir, replace_file_extension(filename, "lir"));
    
    myLIR::opt::optimize(progLir, filename);
    
    myRegAlloc::allocateRegister_x86_64(progLir);  
    myCodeGen::gen_x86_64(progLir);
  } catch(const std::exception& e){
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
