#!/bin/bash
cd ../
mkdir -p bin/
gcc i2c.c examples/i2c-example.c -O2 -o bin/i2c-example
cd examples/
