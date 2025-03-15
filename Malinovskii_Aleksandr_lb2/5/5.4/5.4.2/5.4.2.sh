#!/bin/bash

#cd 5/5.4/5.4.2
# Компиляция программ
gcc -o program1 different_pri1.c
gcc -o program2 different_pri2.c
gcc -o program3 different_pri3.c

# Запуск программ в фоне
taskset -c 0 ./program1 &
taskset -c 0 ./program2 &
taskset -c 0 ./program3 &

# Даем программам поработать некоторое время
sleep 5

# Убить процессы
pkill program1
pkill program2
pkill program3

# Удаление исполняемых файлов
rm program1 program2 program3