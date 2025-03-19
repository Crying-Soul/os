#!/bin/bash

cd 5/5.4/5.4.1
# Компиляция программ
gcc -o program1 program1.c
gcc -o program2 program2.c
gcc -o program3 program3.c

# Запуск программ в фоне
taskset -c 0 ./program1 &
taskset -c 0 ./program2 &
taskset -c 0 ./program3 &

# Даем программам поработать некоторое время
sleep 3

# Убить процессы
pkill program1
pkill program2
pkill program3

# Удаление исполняемых файлов
rm program1 program2 program3