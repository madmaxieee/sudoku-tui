#!/usr/bin/env bash

mkdir -p ./build && cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..

