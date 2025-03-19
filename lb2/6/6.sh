#!/bin/bash

sudo strace -tt -v -s 1000 -e trace=signal ./lb2/6/6 

echo -e "\ntestfile_fork.txt:\n"
cat testfile_fork.txt
echo -e "\ntestfile_exec.txt:\n"
cat testfile_exec.txt
echo -e "\ntestfile_clone:\n"
cat testfile_clone.txt


rm testfile_clone.txt testfile_fork.txt testfile_exec.txt
