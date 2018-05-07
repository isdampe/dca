#!/bin/bash
cd ../
mkdir -p bin/
gcc scheduler.c examples/scheduler-example.c -O2 -o bin/scheduler-example
cd examples/
