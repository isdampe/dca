#!/bin/bash
if [ $# -eq 0 ]
  then
    echo "Syntax: sudo ./serial-monitor.sh [tty.usbmodem1421]"
	exit
fi
while true
do
	sudo screen /dev/$1
	sleep 5
done