#include "ycc.hpp"

void expect(TokenType tk_type){
  auto& token = tokens.front();
  if(token->tokenType != tk_type){
    std::cerr << "not a expected token" << std::endl;
    exit(1);
  }
  tokens.pop();
  return;
}

int expect_number(){
  auto& token = tokens.front();
  if(token->tokenType != TokenType::NUM){
    std::cerr << "not a number" << std::endl;
    exit(1);
  }
  const int retVal = token->value;
  tokens.pop();
  return retVal;
} 

bool consume_symbol(TokenType tk_type){
  auto& token = tokens.front();
  if(token->tokenType != tk_type){
    return false;
  }
  tokens.pop();
  return true;
}

void new_token(TokenType tk_type, int value, const std::string& str){
  auto token = std::make_unique<Token>(tk_type, value, str);
  tokens.push(std::move(token)); 
}


void tokenize(const std::string& input){                           
  std::size_t end = 0;
  
  while(end < input.size()){ 
    std::size_t begin = end;   
    if(std::isspace(input[begin])){ 
      end++; 
      continue;                     
    }
    
    if(std::isdigit(input[begin])){     
      while(std::isdigit(input[end])) ++end;
      new_token(TokenType::NUM, std::stoi(input.substr(begin, end-begin)), "");                                               
      continue;      
    }
    
    if(input[begin] == '+'){
      end++;
      new_token(TokenType::PLUS, 0, std::string(1, input[begin]));
      continue;
    }
    
    if(input[begin] == '-'){         
      end++;
      new_token(TokenType::MINUS, 0, std::string(1, input[begin]));
      continue; 
    }

    if(input[begin] == '*'){                                         
      end++;
      new_token(TokenType::STAR, 0, std::string(1, input[begin]));
      continue;           
    }
    
    if(input[begin] == '/'){             
      end++;
      new_token(TokenType::SLASH, 0, std::string(1, input[begin]));
      continue; 
    }  
  } //while()
  new_token(TokenType::TK_EOF, 0, "");
} //tokenize()
