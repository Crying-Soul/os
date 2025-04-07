#!/bin/bash

# Скрипт для компиляции и тестирования обработки сигналов при fork и exec

# Компиляция программ
gcc signal_test.c -o signal_test || { echo -e "\033[1;31mОшибка компиляции signal_test.c\033[0m"; exit 1; }
gcc exec_program.c -o exec_program || { echo -e "\033[1;31mОшибка компиляции exec_program.c\033[0m"; exit 1; }


# Тестирование fork
echo -e "\n\033[1;33m=== ТЕСТИРОВАНИЕ FORK ===\033[0m"
strace -e trace=signal ./signal_test fork &
parent_pid=$!
sleep 1 # Даем время на запуск

child_pid=$(pgrep -P $parent_pid)


wait $parent_pid # Ожидаем завершения

# Тестирование exec
echo -e "\n\033[1;33m=== ТЕСТИРОВАНИЕ EXEC ===\033[0m"
strace -e trace=signal ./signal_test exec &
parent_pid=$!
sleep 1 # Даем время на запуск

child_pid=$(pgrep -P $parent_pid)

sleep 1 # Даем время на выполнение exec

wait $parent_pid # Ожидаем завершения

rm -f signal_test exec_program
