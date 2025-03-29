#!/bin/bash

cd 1
gcc -o 1.1 1.1.c
gcc -o 1.2 1.2.c
sudo strace -tt -v -s 1000 -e trace=signal ./1.1
echo "=============================================="
sudo strace -tt -v -s 1000 -e trace=signal ./1.2
rm 1.1 1.2