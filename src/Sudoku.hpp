#ifndef _SUDOKU_H_
#define _SUDOKU_H_

#include <ftxui/dom/elements.hpp>
#include <iostream>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace ftxui;
using namespace std;

typedef vector<vector<size_t>> Board;
typedef pair<size_t, size_t> Coord;

enum State {
  UNSURE,
  INVALID,
  FINISHED,
};

const size_t FULL = 0x1ff;

class Sudoku {
public:
  // n is the size of the sudoku board
  Sudoku(size_t n);
  void print() const;
  void play();
  void reset();
  void new_game(size_t cells_given);
  bool is_finished() {
    check_finished();
    return _state == FINISHED;
  }
  bool is_invalid() {
    check_invalid();
    return _state == INVALID;
  }

private:
  Element _document;
  Board _board;
  Board _solution;
  vector<vector<bool>> _is_given;
  size_t _size;
  size_t _size2;
  bool _is_ready;
  State _state;
  vector<size_t> _row_flags;
  vector<size_t> _col_flags;
  vector<size_t> _box_flags;
  // check this cell to check if the board is full and valid
  Coord _empty_cell;
  // invalid groups, (row, col, box)
  tuple<unordered_set<size_t>, unordered_set<size_t>, unordered_set<size_t>>
      _invalid_groups;

  void place_number(Coord coord, size_t num);
  void check_invalid();
  void check_finished();
  bool solve();
  bool _solve(size_t cursor);
};

#endif // !_SUDOKU_H_
