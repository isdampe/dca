#!/bin/bash
if [ $# -eq 0 ]
  then
    echo "Syntax: ./build-flash.sh [device_name]"
	exit
fi
particle flash $1