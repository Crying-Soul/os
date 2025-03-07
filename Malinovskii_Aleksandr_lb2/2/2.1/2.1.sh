#!/bin/bash

cd 2/2.1
gcc -o computations computations.c
gcc -o cycle_computations cycle_computations.c

echo Пример единичный вычислений в родителе и потомке
./computations

echo Пример множественных вычислений в родителе и потомке и демонстрация того что у родителя и потомка разное адресное пространство 
./cycle_computations

rm computations cycle_computations 
