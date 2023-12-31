#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <fmt/color.h>
#include <fmt/format.h>
#include <ftxui/component/component.hpp> // for Button, Vertical, Renderer, Horizontal, operator|
#include <ftxui/component/component_base.hpp>     // for Component
#include <ftxui/component/component_options.hpp>  // for ButtonOption
#include <ftxui/component/screen_interactive.hpp> // for ScreenInteractive
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Sudoku.hpp"

using namespace std;

Sudoku::Sudoku(size_t n) {
  srand(time(nullptr));
  _size = n;
  _size2 = n * n;
  reset();
}

void Sudoku::reset() {
  _board = vector<vector<size_t>>(_size2, vector<size_t>(_size2, 0));
  _labels = vector<vector<string>>(_size2, vector<string>(_size2, ""));
  _is_given = vector<vector<bool>>(_size2, vector<bool>(_size2, false));
  _is_ready = false;
  _state = UNSURE;
  _row_flags = vector<size_t>(_size2, 0);
  _col_flags = vector<size_t>(_size2, 0);
  _box_flags = vector<size_t>(_size2, 0);
  _empty_cell = make_pair(0, 0);
}

void Sudoku::print() const {
  ostringstream border_oss;
  border_oss << ' ';
  for (size_t r = 0; r < (_size * 2 - 1) * _size + 3 * (_size - 1); ++r) {
    border_oss << '-';
  }
  string border = border_oss.str();

  cout << "Board:" << endl;
  for (size_t r = 0; r < _size2; ++r) {
    if (r % _size == 0 && r != 0) {
      cout << border << endl;
    }
    for (size_t c = 0; c < _size2; ++c) {
      if (c % _size == 0 && c != 0) {
        cout << " |";
      }
      if (_is_given[r][c]) {
        fmt::print(fg(fmt::color::red), "{:2d}", _board[r][c]);
      } else {
        fmt::print("{:2d}", _board[r][c]);
      }
    }
    cout << endl;
  }
  cout << endl;
}

void Sudoku::new_game(size_t cells_given) {
  assert(cells_given <= _size2 * _size2 && "Too many cells given");

  auto random = [&]() { return rand() % _size2; };

  auto _new_game = [&]() {
    reset();
    size_t count = 0;
    while (count < cells_given) {
      size_t row = random();
      size_t col = random();
      place_number({row, col}, random() + 1);
      _is_given[row][col] = true;
      if (is_invalid()) {
        place_number({row, col}, 0);
        _is_given[row][col] = false;
      } else {
        count++;
      }
    }
  };

  do {
    _new_game();
  } while (!solve());
}

void Sudoku::check_invalid() {
  if (_state != INVALID) {
    return;
  }

  auto &[row_invalid, col_invalid, box_invalid] = _invalid_groups;
  _state = UNSURE;

  unordered_set<size_t> row_invalid_to_remove;
  for (auto &r : row_invalid) {
    _row_flags[r] = 0;
    bool valid = true;
    for (size_t c = 0; c < _size2; ++c) {
      size_t flag = (1 << _board[r][c]) >> 1;
      if (flag & _row_flags[r]) {
        _state = INVALID;
        valid = false;
      }
      _row_flags[r] |= flag;
    }
    if (valid) {
      row_invalid_to_remove.insert(r);
    }
  }
  for (auto &r : row_invalid_to_remove) {
    row_invalid.erase(r);
  }

  unordered_set<size_t> col_invalid_to_remove;
  for (auto &c : col_invalid) {
    _col_flags[c] = 0;
    bool valid = true;
    for (size_t r = 0; r < _size2; ++r) {
      size_t flag = (1 << _board[r][c]) >> 1;
      if (flag & _col_flags[c]) {
        _state = INVALID;
        valid = false;
      }
      _col_flags[c] |= flag;
    }
    if (valid) {
      col_invalid_to_remove.insert(c);
    }
  }
  for (auto &c : col_invalid_to_remove) {
    col_invalid.erase(c);
  }

  unordered_set<size_t> box_invalid_to_remove;
  for (auto &b : box_invalid) {
    _box_flags[b] = 0;
    size_t row = b / _size * _size;
    size_t col = b % _size * _size;
    bool valid = true;
    for (size_t r = row; r < row + _size; ++r) {
      for (size_t c = col; c < col + _size; ++c) {
        size_t flag = (1 << _board[r][c]) >> 1;
        if (flag & _box_flags[b]) {
          _state = INVALID;
          valid = false;
        }
        _box_flags[b] |= flag;
      }
    }
    if (valid) {
      box_invalid_to_remove.insert(b);
    }
  }
  for (auto &b : box_invalid_to_remove) {
    box_invalid.erase(b);
  }
}

void Sudoku::place_number(Coord coord, size_t num) {
  auto [row, col] = coord;

  size_t orig = _board[row][col];
  size_t orig_flag = (1 << orig) >> 1;
  size_t num_flag = (1 << num) >> 1;

  _board[row][col] = num;
  _labels[row][col] = num == 0 ? "+" : fmt::format("{}", num);

  if (_state == FINISHED) {
    _state = UNSURE;
  }

  _row_flags[row] ^= orig_flag;
  _col_flags[col] ^= orig_flag;
  _box_flags[row / _size * _size + col / _size] ^= orig_flag;

  auto &[row_invalid, col_invalid, box_invalid] = _invalid_groups;
  if (_row_flags[row] & num_flag) {
    _state = INVALID;
    row_invalid.insert(row);
  }
  if (_col_flags[col] & num_flag) {
    _state = INVALID;
    col_invalid.insert(col);
  }
  if (_box_flags[row / _size * _size + col / _size] & num_flag) {
    _state = INVALID;
    box_invalid.insert(row / _size * _size + col / _size);
  }

  _row_flags[row] |= num_flag;
  _col_flags[col] |= num_flag;
  _box_flags[row / _size * _size + col / _size] |= num_flag;
}

void Sudoku::check_finished() {
  if (_state == FINISHED) {
    return;
  }

  auto &[r, c] = _empty_cell;
  if (_board[r][c] == 0) {
    return;
  }

  for (size_t cursor = 0; cursor < _size2 * _size2; cursor++) {
    size_t row = cursor / _size2;
    size_t col = cursor % _size2;
    if (_board[row][col] == 0) {
      _empty_cell = make_pair(row, col);
      return;
    }
  }

  if (!is_invalid()) {
    _state = FINISHED;
  }
}

bool Sudoku::solve() {
  bool solved = _solve(0);

  // clean up
  if (solved) {
    // recalculate flags
    _row_flags = vector<size_t>(_size2, 0);
    _col_flags = vector<size_t>(_size2, 0);
    _box_flags = vector<size_t>(_size2, 0);
    _invalid_groups =
        make_tuple(unordered_set<size_t>(), unordered_set<size_t>(),
                   unordered_set<size_t>());
    _solution = vector<vector<size_t>>(_size2, vector<size_t>(_size2, 0));

    for (size_t cursor = 0; cursor < _size2 * _size2; cursor++) {
      size_t row = cursor / _size2;
      size_t col = cursor % _size2;
      _solution[row][col] = _board[row][col];
      if (_is_given[row][col]) {
        size_t num = _solution[row][col];
        size_t num_flag = (1 << num) >> 1;
        _row_flags[row] |= num_flag;
        _col_flags[col] |= num_flag;
        _box_flags[row / _size * _size + col / _size] |= num_flag;
      } else {
        // _board[row][col] = 0;
        place_number({row, col}, 0);
      }
    }
  }

  return solved;
}

bool Sudoku::_solve(size_t cursor) {
  if (cursor == _size2 * _size2) {
    return true;
  }

  size_t row = cursor / _size2;
  size_t col = cursor % _size2;

  if (_is_given[row][col]) {
    return _solve(cursor + 1);
  }

  for (size_t n = 1; n <= _size2; n++) {
    place_number({row, col}, n);
    if (is_invalid()) {
      continue;
    }
    if (_solve(cursor + 1)) {
      return true;
    }
  }
  place_number({row, col}, 0);
  return false;
};

void Sudoku::play_text() {
  auto get_input = [&](string prompt, size_t min, size_t max) {
    while (true) {
      cout << prompt;
      string input;
      cin >> input;
      // try to convert to size_t
      try {
        size_t num = stoul(input);
        if (num > max || num < min) {
          throw invalid_argument("invalid input");
        }
        return num;
      } catch (invalid_argument &e) {
        cerr << "Error: invalid input. Expected a number between 1 and 9. Got "
             << input << "." << endl;
      }
    }
  };

  while (!is_finished()) {
    print();
    size_t row = get_input("Enter row: ", 1, _size2) - 1;
    size_t col = get_input("Enter column: ", 1, _size2) - 1;
    size_t num = get_input("Enter number (enter 0) to clear: ", 0, _size2);
    if (_is_given[row][col]) {
      cerr << "Error: cannot change given number." << endl;
      continue;
    }
    place_number({row, col}, num);
  }

  cout << "Congratulations! You solved the puzzle!" << endl;
};

void Sudoku::play_tui() {
  using namespace ftxui;

  Coord cursor = make_pair(0, 0);
  size_t count = 0;

  auto cell = [&](size_t r, size_t c) {
    size_t num = _board[r][c];
    auto button = Button(
        &_labels[r][c], [&cursor, r, c] { cursor = make_pair(r, c); },
        ButtonOption::Ascii());
    if (_is_given[r][c]) {
      return button | color(Color::Red1);
    } else {
      return button;
    }
  };

  vector<Component> board_content;
  for (size_t r = 0; r < _size2; ++r) {
    if (r % _size == 0 && r != 0) {
      board_content.push_back(Renderer([] { return separator(); }));
    }
    vector<Component> row_content;
    for (size_t c = 0; c < _size2; ++c) {
      if (c % _size == 0 && c != 0) {
        row_content.push_back(Renderer([] { return separator(); }));
      }
      row_content.push_back(cell(r, c));
    }
    board_content.push_back(Container::Horizontal(row_content));
  }
  Component board = Container::Vertical(board_content) | border;

  auto num_button = [&](size_t n) {
    return Button(
               fmt::format("{}", n),
               [n, &cursor, this] {
                 auto [r, c] = cursor;
                 if (!this->_is_given[r][c])
                   this->place_number(cursor, n);
               },
               ButtonOption::Ascii()) |
           border;
  };
  Component clear_button = Button(
                               "clear",
                               [&cursor, this] {
                                 auto [r, c] = cursor;
                                 if (!this->_is_given[r][c])
                                   this->place_number(cursor, 0);
                               },
                               ButtonOption::Ascii()) |
                           hcenter | border;

  vector<Component> numpad_content;
  for (size_t i = 0; i < _size; i++) {
    vector<Component> numpad_row_content;
    for (size_t j = 0; j < _size; j++) {
      numpad_row_content.push_back(num_button(i * _size + j + 1));
    }
    numpad_content.push_back(Container::Horizontal(numpad_row_content));
  }
  numpad_content.push_back(clear_button);
  Component numpad = Container::Vertical(numpad_content);

  Component win_window =
      Renderer([&] {
        return window(text("You won!"),
                      text("Congratulations, You solved the puzzle!"));
      }) |
      Maybe([this] { return this->is_finished(); });

  auto screen = ScreenInteractive::FitComponent();
  screen.Loop(Container::Vertical({
      Container::Horizontal({board, numpad | vcenter}),
      win_window,
  }));
}
