#!/bin/bash


echo "=============================================="
echo "                  ЗАДАНИЕ 3.1                 "
echo "=============================================="
strace -tt -e 'trace=signal,kill,tgkill,tkill,sigaction,sigprocmask' -f ./lb3/3/3.1

echo -e "\n=============================================="
echo "                  ЗАДАНИЕ 3.2                 "
echo "=============================================="

strace -tt -e 'trace=signal,kill,tgkill,tkill,sigaction,sigprocmask' -f ./lb3/3/3.2
