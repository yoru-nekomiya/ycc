#include "ycc.hpp"

struct Config {
  bool opt = false;
  bool emit_lir = false;
  bool emit_cfg = false;
  std::string input_file;
};

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

static void print_usage(const std::string& cc){
  std::cerr << "Usage: " << cc << " [options] <input_file>\n"
	    << "Options:\n"
	    << "  -opt        Optimize <input_file>\n"
	    << "  -emit-lir   Dump Low-level IR\n"
	    << "  -emit-cfg   Dump Control Flow Graph (dot format)\n";
}

int main(int argc, char* argv[]){
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  if(argc < 2){
    print_usage(argv[0]);
    return 1;
  }

  //input analysis
  Config config;
  std::vector<std::string> args(argv+1, argv+argc);
  for(int i = 0; i < args.size(); i++){
    std::string_view arg = args[i];
    
    if(arg == "-emit-lir"){
      config.emit_lir = true;
    } 
    else if(arg == "-emit-cfg"){
      config.emit_cfg = true;
    }
    else if(arg == "-opt"){
      config.opt = true;
    }
    else if(arg.starts_with("-")){
      std::cerr << "Unknown option: " << arg << std::endl;
      return 1;
    } 
    else{
      config.input_file = arg;
    }
  }
  
  try{
    //const std::string filename = argv[1];
    const std::string input = readFileToString(config.input_file);
    myTokenizer::tokenize(input);
    auto prog = myParser::program();
    auto progHir = myHIR::generateHirNode(prog);
    
    auto progLir = myLIR::generateLirNode(progHir);
    
    myLIR::opt::optimize(progLir, config.input_file, config.opt, config.emit_cfg);
    if(config.emit_lir){
      dumpLIR(progLir, replace_file_extension(config.input_file, "lir"));
    }
    myRegAlloc::allocateRegister_x86_64(progLir);  
    myCodeGen::gen_x86_64(progLir);
  } catch(const std::exception& e){
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
