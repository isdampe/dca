#!/bin/bash
cd ../
mkdir -p bin/
gcc algorithm.c examples/algorithm-example.c -o bin/algorithm-example -lm
cd examples/
