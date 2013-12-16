#!/bin/bash

gtkterm -p /dev/ttyS1 -s 115200 &

TID=$! 

#read blaj
sleep 2

echo "a" > /dev/ttyS1
sleep 0.01

echo "loads" > /dev/ttyS1
sleep 0.1

cat prog_arm_bb.srec > /dev/ttyS1
sleep 1

echo "go 80000000" > /dev/ttyS1
sleep 1


kill $TID
sleep 2

python tcp_serial_redirect_mod.py -p "/dev/ttyS1" -b115200 -P2546 

echo $!

