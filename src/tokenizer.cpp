#include "ycc.hpp"

namespace myTokenizer {
  std::deque<std::unique_ptr<Token>> tokens = {};
  
void expect(TokenType tk_type){
  auto& token = tokens.front();
  if(token->tokenType != tk_type){
    std::cerr << "not a expected token" << std::endl;
    exit(1);
  }
  tokens.pop_front();
  return;
}

int expect_number(){
  auto& token = tokens.front();
  if(token->tokenType != TokenType::NUM){
    std::cerr << "not a number" << std::endl;
    exit(1);
  }
  const int retVal = token->value;
  tokens.pop_front();
  return retVal;
}

std::string expect_ident(){
  auto& token = tokens.front();
  if(token->tokenType != TokenType::IDENT){
    std::cerr << "not an identifier" << std::endl;
    exit(1);
  }
  const auto str = token->str;
  tokens.pop_front();
  return str;
}

bool consume_symbol(TokenType tk_type){
  auto& token = tokens.front();
  if(token->tokenType != tk_type){
    return false;
  }
  tokens.pop_front();
  return true;
}

std::unique_ptr<Token> consume_ident(){
  auto& token = tokens.front();
  if(token->tokenType != TokenType::IDENT){
    return nullptr;
  }
  auto ret = std::move(tokens.front());
  tokens.pop_front();
  return ret;
}

  std::unique_ptr<Token> consume_str(){
    auto& token = tokens.front();
    if(token->tokenType != TokenType::STR){
      return nullptr;
    }
    auto ret = std::move(tokens.front());
    tokens.pop_front();
    return ret;
  }

  bool look(TokenType tk_type){
    auto& token = tokens.front();
    if(token->tokenType != tk_type){
      return false;
    }
    return true;
  }

static void new_token(TokenType tk_type,
		      unsigned long long value = 0,
		      const std::string& str = "",
		      const std::string& literal = ""){
  auto token = std::make_unique<Token>(tk_type, value, str, literal);
  tokens.push_back(std::move(token)); 
}

bool at_eof(){
  return tokens.front()->tokenType == TokenType::TK_EOF;
}

static TokenType starts_keyword(const std::string& str){
  const std::vector<std::string> keyword =
    {"return","if","else","while","for",
     "int","sizeof","char","short","long",
     "void","break","continue","switch","case",
     "goto","default","do"};  
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
      case 5:
	return TokenType::INT;
      case 6:
	return TokenType::SIZEOF;
      case 7:
	return TokenType::CHAR;
      case 8:
	return TokenType::SHORT;
      case 9:
	return TokenType::LONG;
      case 10:
	return TokenType::VOID;
      }
    }
    i++;
  }
  return TokenType::TK_EOF;
}

  static char escape_char(char c){
    switch(c){
    case 'n': return '\n'; 
    case 't': return '\t'; 
    case 'r': return '\r';
    case '\\': return '\\'; 
    case '\'': return '\''; 
    case '\"': return '\"'; 
    case '0': return '\0'; 
    case 'v': return '\v'; 
    case 'b': return '\b'; 
    case 'f': return '\f'; 
    case 'a': return '\a'; 
    case '?': return '\?'; 
    }
    std::cerr << "unknown escape character" << std::endl;
    exit(1);
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
      new_token(TokenType::NUM, std::stoull(input.substr(begin, end-begin)));
      continue;      
    }
    
    if(c == '+'){
      if(input[end] == '+'){
	end++;
	new_token(TokenType::PLUSPLUS);
      } else {
	new_token(TokenType::PLUS);
      }
      continue;
    }
    
    if(c == '-'){
      if(input[end] == '-'){
	end++;
	new_token(TokenType::MINUSMINUS);
      } else {
	new_token(TokenType::MINUS);
      }
      continue; 
    }

    if(c == '*'){                                         
      new_token(TokenType::STAR);
      continue;           
    }
    
    if(c == '/'){
      if(input[end] == '/'){
	while(input[end] != '\n') ++end;
	++end;
      } else if(input[end] == '*'){
	++end;
	while(true){
	  if(input[end] == EOF){
	    std::cerr << "unterminated block comment" << std::endl;
	    exit(1);
	  }
	  if(input[end] == '*' && input[end+1] == '/'){
	    end += 2;
	    break;
	  }
	  ++end;
	}
      }
      else {
	new_token(TokenType::SLASH);
      }
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

    if(c == '['){ 
      new_token(TokenType::BRACKET_L);
      continue; 
    }

    if(c == ']'){ 
      new_token(TokenType::BRACKET_R);
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

    if(c == ','){
      new_token(TokenType::COMMA);
      continue; 
    }

    if(c == '&'){
      if(input[end] == '&'){
	end++;
	new_token(TokenType::ANDAND);
      } else {
	new_token(TokenType::AND);
      }
      continue; 
    }

    if(c == '|'){
      if(input[end] == '|'){
	end++;
	new_token(TokenType::OROR);
      } else {
	new_token(TokenType::OR);
      }
      continue; 
    }

    if(c == '\''){
      char c = 0;
      if(input[end] == '\\'){
	end++;
	c = escape_char(input[end]);	
      } else {
	c = input[end];
      }
      end++;
      if(input[end] != '\''){
	std::cerr << "missing closing quote for character literal" << std::endl;
	exit(1);
      }
      end++;
      new_token(TokenType::NUM, c);
      continue;
    }

    if(c == '"'){
      std::string str = "";
      while(input[end] != '"'){
	if(input[end] == '\\'){
	  end++;
	  str += escape_char(input[end]);
	} else {
	  str += input[end];
	}
	end++;
      } //while
      end++;
      str += '\0';
      new_token(TokenType::STR, 0, "", str);
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
} //namespace myTokenizer
