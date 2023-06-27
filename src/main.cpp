#include "Sudoku.hpp"

int main(void) {
  Sudoku sudoku = Sudoku(3);
  sudoku.new_game(10);
  sudoku.play_text();

  return EXIT_SUCCESS;
}
