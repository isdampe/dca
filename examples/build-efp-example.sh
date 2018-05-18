#!/bin/bash
cd ../
mkdir -p bin/
gcc i2c.c efp.c examples/efp-example.c -o bin/efp-example
cd examples/
