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

static void new_token(TokenType tk_type, int value = 0){
  auto token = std::make_unique<Token>(tk_type, value);
  tokens.push(std::move(token)); 
}


void tokenize(const std::string& input){                           
  std::size_t end = 0;
  
  while(end < input.size()){ 
    const std::size_t begin = end;
    const char& c = input[begin];
    if(std::isspace(c)){ 
      end++; 
      continue;                     
    }
    
    if(std::isdigit(c)){     
      while(std::isdigit(input[end])) ++end;
      new_token(TokenType::NUM, std::stoi(input.substr(begin, end-begin)));                                               
      continue;      
    }
    
    if(c == '+'){
      end++;
      new_token(TokenType::PLUS);
      continue;
    }
    
    if(c == '-'){         
      end++;
      new_token(TokenType::MINUS);
      continue; 
    }

    if(c == '*'){                                         
      end++;
      new_token(TokenType::STAR);
      continue;           
    }
    
    if(c == '/'){             
      end++;
      new_token(TokenType::SLASH);
      continue; 
    }

    if(c == '('){             
      end++;
      new_token(TokenType::PAREN_L);
      continue; 
    }

    if(c == ')'){             
      end++;
      new_token(TokenType::PAREN_R);
      continue; 
    }

    if(c == '<'){
      end++;
      if(input[end] == '='){
	end++;
	new_token(TokenType::LE);
      } else {
	new_token(TokenType::LT);
      }
      continue;
    }

    if(c == '>'){
      end++;
      if(input[end] == '='){
	end++;
	new_token(TokenType::GE);
      } else {
	new_token(TokenType::GT);
      }
      continue;
    }

    if(c == '='){
      end++;
      if(input[end] == '='){
	end++;
	new_token(TokenType::EQ);
      } else {
	new_token(TokenType::ASSIGN);
      }
      continue;
    }

    if(c == '!'){
      end++;
      if(input[end] == '='){
	end++;
	new_token(TokenType::NE);
      } else {
	new_token(TokenType::NOT);
      }
      continue;
    }
    
    std::cerr << "invalid token" << std::endl;
    exit(1);
  } //while()
  new_token(TokenType::TK_EOF);
} //tokenize()
