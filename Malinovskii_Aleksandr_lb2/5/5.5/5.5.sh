#!/bin/bash

# Компиляция программ (если они еще не скомпилированы)
echo "Компиляция программ..."
gcc -o rr_program rr.c
gcc -o fifo_program fifo.c
gcc -o other_program other.c

# Запуск программ в фоновом режиме
echo "Запуск программ..."
sudo ./rr_program &
sudo ./fifo_program &
sudo ./other_program &

# Ждем 10 секунд (или другое время) перед завершением
echo "Ожидание 10 секунд..."
sleep 10

pkill rr_program
pkill fifo_program
pkill other_program

# Очистка скомпилированных файлов
rm -f rr_program fifo_program other_program
