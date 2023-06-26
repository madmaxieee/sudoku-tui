#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <iostream>
#include <unordered_set>
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
  _is_given = vector<vector<bool>>(_size2, vector<bool>(_size2, false));
  _is_ready = false;
  _state = UNSURE;
  _row_flags = vector<size_t>(_size2, 0);
  _col_flags = vector<size_t>(_size2, 0);
  _box_flags = vector<size_t>(_size2, 0);
  _empty_cell = make_pair(0, 0);
}

void Sudoku::print() const {
  cout << "Board:" << endl;
  for (auto &row : _board) {
    for (auto &n : row) {
      cout << n << ' ';
    }
    cout << endl;
  }
  cout << endl;
}

void Sudoku::new_game(size_t cells_given) {
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
    size_t row = b / 3 * 3;
    size_t col = b % 3 * 3;
    bool valid = true;
    for (size_t r = row; r < row + 3; ++r) {
      for (size_t c = col; c < col + 3; ++c) {
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

  if (_state == FINISHED) {
    _state = UNSURE;
  }

  _row_flags[row] ^= orig_flag;
  _col_flags[col] ^= orig_flag;
  _box_flags[row / 3 * 3 + col / 3] ^= orig_flag;

  auto &[row_invalid, col_invalid, box_invalid] = _invalid_groups;
  if (_row_flags[row] & num_flag) {
    _state = INVALID;
    row_invalid.insert(row);
  }
  if (_col_flags[col] & num_flag) {
    _state = INVALID;
    col_invalid.insert(col);
  }
  if (_box_flags[row / 3 * 3 + col / 3] & num_flag) {
    _state = INVALID;
    box_invalid.insert(row / 3 * 3 + col / 3);
  }

  _row_flags[row] |= num_flag;
  _col_flags[col] |= num_flag;
  _box_flags[row / 3 * 3 + col / 3] |= num_flag;
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
        _box_flags[row / 3 * 3 + col / 3] |= num_flag;
      } else {
        _board[row][col] = 0;
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

  for (size_t n = 1; n <= 9; n++) {
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
