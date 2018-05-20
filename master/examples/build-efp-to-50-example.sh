#!/bin/bash
cd ../
mkdir -p bin/
gcc i2c.c efp.c examples/efp-to-50-example.c -o bin/efp-to-50-example
cd examples/
