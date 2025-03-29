#!/bin/bash

cd 4
gcc -o 4 4.c
sudo strace -tt -v -s 1000 -e trace=signal ./4
rm 4