#!/bin/bash

cd 2
gcc -o 2 2.c
sudo strace -tt -v -s 1000 -e trace=signal ./2
rm 2