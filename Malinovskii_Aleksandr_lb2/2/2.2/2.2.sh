#!/bin/bash

cd 2/2.2
g++ -o father father.cpp
g++ -o son son.cpp

./father
rm father son

echo "Процессы при запуске отца"
cat log_father.txt

echo "Процессы при запуске сынв"
cat log_son.txt

rm log_son.txt log_father.txt