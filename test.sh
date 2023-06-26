#!/usr/bin/env bash

mkdir -p ./build
cd build || exit
make
./sudoku-tui
