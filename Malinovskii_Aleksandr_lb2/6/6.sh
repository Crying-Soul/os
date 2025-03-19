#!/bin/bash

cd 6
gcc -o 6 6.c
sudo strace -tt -v -s 1000 -e trace=signal ./6 
rm ./6

echo -e "\ntestfile_fork.txt:\n"
cat testfile_fork.txt
echo -e "\ntestfile_exec.txt:\n"
cat testfile_exec.txt
echo -e "\ntestfile_clone:\n"
cat testfile_clone_fd.txt


rm testfile_clone_fd.txt testfile_fork.txt testfile_exec.txt