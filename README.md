# Distributed Computational Algorithms on Embedded Systems

## master/

To be run on the computational master, running linux, with an I2C bus available
and loaded into the kernel.

For the terminal interface, the `ncurses` library must be present.

```bash
sudo apt-get install libncurses-dev
```

## photon/

Inside `src/` is all required source code for a Photon Cli project to compile.

## mbed/

Inside `dca-slave/` is a single source file required. The mBed `I2CSlave` library
must also be imported in the Mbed project for compilation.
