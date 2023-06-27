#include "Sudoku.hpp"
#include <string> // for operator+, to_string

int main(void) {
  Sudoku sudoku = Sudoku(3);
  sudoku.new_game(30);
  sudoku.play_tui();

  return EXIT_SUCCESS;
}
