//#include <stdio.h>
//#include <time.h>

int printf();

//int RUNS = 10000000;
#define RUNS 10000000

//char *EXPRESSION = "10 + 2 * (100 - 5 * 3) / 5"; 
char *EXPRESSION = "100 + 5 * (20 * 2) - 50 * (100 / 25) + 3000 / (10 + 5) * 10 + 1000 * (1 + 2)";

char *current_pos;

int parse_expression();
int parse_term();
int parse_factor();

int parse_expression() {
  int result = parse_term();
  
  while (*current_pos == '+' || *current_pos == '-') {
    char op = *current_pos;
    current_pos++;
    int next_term = parse_term();
    
    if (op == '+') {
      result += next_term;
    } else {
      result -= next_term;
    }
  }
  return result;
}

int parse_term() {
  int result = parse_factor();
  
  while (*current_pos == '*' || *current_pos == '/') {
    char op = *current_pos;
    current_pos++;
    int next_factor = parse_factor();
    
    if (op == '*') {
      result *= next_factor;
    } else {
      result /= next_factor;
    }
  }
  return result;
}

int parse_factor() {
  int result = 0;
  
  if (*current_pos >= '0' && *current_pos <= '9') {
    while (*current_pos >= '0' && *current_pos <= '9') {
      result = result * 10 + (*current_pos - '0');
      current_pos++;
    }
    return result;
  } 
  
  else if (*current_pos == '(') {
    current_pos++; 
    result = parse_expression(); 
    
    if (*current_pos == ')') {
      current_pos++; 
    }
    return result;
  }
  
  return 0; 
}

int main() {
  current_pos = EXPRESSION;
  int result;  
  
  for(int i=0; i<RUNS; i++) {
    current_pos = EXPRESSION;
    result = parse_expression();
  }
  printf("Final Result (Checksum): %d\n", result);
  return 0;
}
