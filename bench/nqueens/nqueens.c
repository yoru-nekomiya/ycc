//#include <stdio.h>
//#include <time.h>

int printf();

//int BOARD_SIZE = 12;
#define BOARD_SIZE 12

int solutions_count = 0;
int board[BOARD_SIZE/*12*/]; 

int is_safe(int row, int col) {
  int prev_row;
  for (prev_row = 0; prev_row < row; prev_row++) {
    if (board[prev_row] == col || 
	(board[prev_row] - col) == (prev_row - row) || 
	(board[prev_row] - col) == (row - prev_row)) {
      return 0;
    }
  }
  return 1;
}

void solve_queens(int row) {
  if (row == BOARD_SIZE) {
    solutions_count++;
    return;
  }
  
  int col;
  for (col = 0; col < BOARD_SIZE; col++) {
    if (is_safe(row, col)) {
      board[row] = col;
      solve_queens(row + 1);
    }
  }
}

int main() {
  for(int i=0; i<BOARD_SIZE; i++)
    board[i] = 0;
    
  solve_queens(0);
    
  printf("N-Queens (Size: %d), ", BOARD_SIZE);
  printf("Solutions Found: %d\n", solutions_count); 
  return 0;
}
