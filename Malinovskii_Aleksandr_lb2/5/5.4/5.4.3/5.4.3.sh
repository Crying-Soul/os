#!/bin/bash

cd 5/5.4/5.4.3
# Компиляция программ
gcc yield1.c -o rr_program1
gcc yield2.c -o rr_program2
gcc yield3.c -o rr_program3

# Запуск программ в фоне
sudo taskset -c 0 ./rr_program1 &
sudo taskset -c 0 ./rr_program2 &
sudo taskset -c 0 ./rr_program3 &

sleep 7
pkill rr_program1
pkill rr_program2
pkill rr_program3

rm rr_program*

echo "Все программы завершены."