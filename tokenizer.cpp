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

std::unique_ptr<Token> consume_ident(){
  auto& token = tokens.front();
  if(token->tokenType != TokenType::IDENT){
    return nullptr;
  }
  auto ret = std::move(tokens.front());
  tokens.pop();
  return ret;
}

static void new_token(TokenType tk_type,
		      int value = 0,
		      const std::string& str = ""){
  auto token = std::make_unique<Token>(tk_type, value, str);
  tokens.push(std::move(token)); 
}

bool at_eof(){
  return tokens.front()->tokenType == TokenType::TK_EOF;
}

static TokenType starts_keyword(const std::string& str){
  const std::vector<std::string> keyword =
    {"return","if","else","while","for",
     "int","char","short","long","void",
     "break","continue","switch","case","goto",
     "default","do"};  
  int i = 0;
  for(const auto& kw: keyword){
    if(str == kw){
      switch(i){
      case 0:
	return TokenType::RETURN;
      case 1:
	return TokenType::IF;
      case 2:
	return TokenType::ELSE;
      case 3:
	return TokenType::WHILE;
      case 4:
	return TokenType::FOR;
      }
    }
    i++;
  }
  return TokenType::TK_EOF;
}


void tokenize(const std::string& input){                           
  std::size_t end = 0;
  
  while(end < input.size()){ 
    const std::size_t begin = end;
    const char& c = input[begin];
    end++;
    if(std::isspace(c)){ 
      continue;                     
    }
    
    if(std::isdigit(c)){     
      while(std::isdigit(input[end])) ++end;
      new_token(TokenType::NUM, std::stoi(input.substr(begin, end-begin)));                                               
      continue;      
    }
    
    if(c == '+'){
      new_token(TokenType::PLUS);
      continue;
    }
    
    if(c == '-'){         
      new_token(TokenType::MINUS);
      continue; 
    }

    if(c == '*'){                                         
      new_token(TokenType::STAR);
      continue;           
    }
    
    if(c == '/'){             
      new_token(TokenType::SLASH);
      continue; 
    }

    if(c == '('){             
      new_token(TokenType::PAREN_L);
      continue; 
    }

    if(c == ')'){             
      new_token(TokenType::PAREN_R);
      continue; 
    }

    if(c == '{'){ 
      new_token(TokenType::BRACE_L);
      continue; 
    }

    if(c == '}'){ 
      new_token(TokenType::BRACE_R);
      continue; 
    }

    if(c == '<'){
      if(input[end] == '='){
	end++;
	new_token(TokenType::LE);
      } else {
	new_token(TokenType::LT);
      }
      continue;
    }

    if(c == '>'){
      if(input[end] == '='){
	end++;
	new_token(TokenType::GE);
      } else {
	new_token(TokenType::GT);
      }
      continue;
    }

    if(c == '='){
      if(input[end] == '='){
	end++;
	new_token(TokenType::EQ);
      } else {
	new_token(TokenType::ASSIGN);
      }
      continue;
    }

    if(c == '!'){
      if(input[end] == '='){
	end++;
	new_token(TokenType::NE);
      } else {
	new_token(TokenType::NOT);
      }
      continue;
    }

    if(c == ';'){
      new_token(TokenType::SEMICOLON);
      continue; 
    }
    
    //identifier or keyword
    if(std::isalpha(c) || c == '_'){
      while(std::isalnum(input[end]) || input[end] == '_'){
	end++;
      }
      const auto word = input.substr(begin, end-begin);
      const TokenType tokenType = starts_keyword(word);
      if(tokenType != TokenType::TK_EOF){
	//keyword
	new_token(tokenType);
      } else {
	//identifier
	new_token(TokenType::IDENT, 0, word);
      }
      continue;
    }
    
    std::cerr << "invalid token" << std::endl;
    exit(1);
  } //while()
  new_token(TokenType::TK_EOF);
} //tokenize()
