#!/usr/bin/env bash

mkdir -p ./build
cd build || exit
cmake ..
make
./sudoku-tui
