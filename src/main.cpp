#include "Sudoku.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <iostream>

int main(void) {
  Sudoku game = Sudoku(2);
  game.new_game(10);

  game.play();

  return 0;

  using namespace ftxui;

  // Define the document
  Element document = hbox({
      text("left") | border,
      text("middle") | border | flex,
      text("right") | border,
  });

  auto screen = Screen::Create(Dimension::Full(),       // Width
                               Dimension::Fit(document) // Height
  );
  Render(screen, document);
  screen.Print();

  return EXIT_SUCCESS;
}
