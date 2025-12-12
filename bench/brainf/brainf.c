//Brainf*ck
//#include <stdio.h>
//#include <time.h>

int printf();

int DATA_SIZE = 30000;
char data_mem[30000]; 
int instruction_count = 0;

char* PROGRAM = 
    "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<<.+++.------.<-.>>+.";

void run_bf(char* program) {
  int pc = 0;
  int data_ptr = 0;
  int loop_count = 0;
  
  for(int i=0; i<DATA_SIZE; i++)
    data_mem[i] = 0;
  
  while(program[pc] != 0) {
    instruction_count++;
    char instruction = program[pc];
    
    if (instruction == '>') {
      data_ptr++;
    } else if (instruction == '<') {
      data_ptr--;
    } else if (instruction == '+') {
      data_mem[data_ptr]++;
    } else if (instruction == '-') {
      data_mem[data_ptr]--;
    } else if (instruction == '[') {
      if (data_mem[data_ptr] == 0) {	
	loop_count = 1;
	while (loop_count > 0) {
	  pc++;
	  if (program[pc] == '[') loop_count++;
	  else if (program[pc] == ']') loop_count--;
	}
      }
    } else if (instruction == ']') {
      if (data_mem[data_ptr] != 0) {
	loop_count = 1;
	while (loop_count > 0) {
	  pc--;
	  if (program[pc] == ']') loop_count++;
	  else if (program[pc] == '[') loop_count--;
	}
      }
    } 
    pc++;
  }
}

int main() {    
  int RUNS = 10000;
  for(int i=0; i<RUNS; i++) {
    run_bf(PROGRAM);
  }  

  long checksum = 0;
  for(int i=0; i<DATA_SIZE; i++)
    checksum += data_mem[i];
  
  printf("Brainf*ck Interpreter (Runs: %d), ", RUNS);
  printf("Total Instructions: %d, ", instruction_count);
  printf("Memory Checksum: %ld\n", checksum);
  return 0;
}
