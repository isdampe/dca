#!/bin/bash
cd ../
mkdir -p bin/
gcc i2c.c efp.c examples/efp-to-50-example-mbed.c -o bin/efp-to-50-example-mbed
gcc i2c.c efp.c examples/efp-to-50-example-photon.c -o bin/efp-to-50-example-photon
cd examples/
