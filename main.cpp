#include <iostream>
#include <string>
#include <queue>
#include <memory>

enum class TokenType {
  NUM,
  SYM,
};

struct Token {
  TokenType tokenType;
  int value;
  std::string str;

  Token(TokenType _tokenType, int _value, const std::string& _str)
    : tokenType(_tokenType), value(_value), str(_str)
  {}
};

std::queue<std::unique_ptr<Token>> tokens = {};

void tokenize(const std::string& input){
  std::size_t end = 0;
  std::unique_ptr<Token> token;
  
  while(end < input.size()){
    std::size_t begin = end;
    if(std::isspace(input[begin])){
      end++;
      continue;
    }

    if(std::isdigit(input[begin])){
      while(std::isdigit(input[end])) ++end;
      token = std::make_unique<Token>(TokenType::NUM,
				      std::stoi(input.substr(begin, end-begin)),
				      "");
      tokens.push(std::move(token));
      continue;
    }

    if(input[begin] == '+' || input[begin] == '-'){
      end++;
      
      token = std::make_unique<Token>(TokenType::SYM,
                                      0,
                                      std::string(1, input[begin]));
      
      tokens.push(std::move(token));
      continue;
    }
    
  } //while()
} //tokenize()

int expect_number(){
  auto& token = tokens.front();
  if(token->tokenType != TokenType::NUM){
    std::cerr << "not a number" << std::endl;
    exit(1);
  }
  int retVal = token->value;
  tokens.pop();
  return retVal;
}

bool consume_symbol(const std::string sym){
  auto& token = tokens.front();
  if(token->tokenType != TokenType::SYM || token->str != sym){
    return false;
  }
  tokens.pop();
  return true;
}

int main(int argc, char* argv[]){
  const std::string input = argv[1];
  tokenize(input);
  
  std::cout << ".intel_syntax noprefix" << std::endl
	    << ".global main" << std::endl
	    << "main:" << std::endl;
  
  std::cout << "  mov rax, " << expect_number() << std::endl;
  
  while(!tokens.empty()){
    if(consume_symbol("+")){
      std::cout << "  add rax, " << expect_number() << std::endl;
      continue;
    }
    consume_symbol("-");
    std::cout << "  sub rax, " << expect_number() << std::endl;
  } //while
  std::cout << "  ret" << std::endl;
  return 0;
}
