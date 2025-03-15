#!/bin/bash

gcc -o fifo1 fifo1.c
gcc -o fifo2 fifo2.c
gcc -o fifo3 fifo3.c
gcc -o 5.3.2 5.3.2.c

# Пути к исполняемым файлам
EXECUTABLES=("fifo1" "fifo2" "fifo3")

# Приоритет для запуска с одинаковыми приоритетами
SAME_PRIORITY=50

# Приоритеты для запуска с разными приоритетами (для каждого из 3-х процессов)
DIFFERENT_PRIORITIES=(30 99 70)

# Функция для ожидания завершения группы процессов
wait_for_processes() {
    for pid in "$@"; do
        wait "$pid"
    done
}
echo "==========================5.3.1=================================="
echo "Запуск тестов с одинаковыми приоритетами ($SAME_PRIORITY)..."
pids=()
for exe in "${EXECUTABLES[@]}"; do
    echo "Запуск $exe с приоритетом $SAME_PRIORITY"
    sudo taskset -c 0 "./$exe" "$SAME_PRIORITY" &
    pids+=($!)
done
wait_for_processes "${pids[@]}"

echo "==========================5.3.2=================================="

./5.3.2

echo "==========================5.3.3=================================="

echo "Запуск тестов с разными приоритетами..."
pids=()
for i in "${!EXECUTABLES[@]}"; do
    exe="${EXECUTABLES[$i]}"
    prio="${DIFFERENT_PRIORITIES[$i]}"
    echo "Запуск $exe с приоритетом $prio"
    sudo taskset -c 0 "./$exe" "$prio" &
    pids+=($!)
done
wait_for_processes "${pids[@]}"

rm fifo1 fifo2 fifo3 5.3.2