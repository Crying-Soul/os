#!/bin/bash

# Пути к исполняемым файлам
EXECUTABLES=("lb2/5/5.3/5.3.3/5.3.3-prog1" "lb2/5/5.3/5.3.3/5.3.3-prog2" "lb2/5/5.3/5.3.3/5.3.3-prog3")

# Приоритет для запуска с одинаковыми приоритетами
SAME_PRIORITY=30

# Приоритеты для запуска с разными приоритетами (для каждого из 3-х процессов)
DIFFERENT_PRIORITIES=(20 99 40)

# Функция для ожидания завершения группы процессов
wait_for_processes() {
    for pid in "$@"; do
        wait "$pid"
    done
}

echo "Запуск тестов с одинаковыми приоритетами ($SAME_PRIORITY)..."
pids=()
for exe in "${EXECUTABLES[@]}"; do
    echo "Запуск $exe с приоритетом $SAME_PRIORITY"
    sudo taskset -c 0 "$exe" "$SAME_PRIORITY" &
    pids+=($!)
done
wait_for_processes "${pids[@]}"

echo "---------------------------------------------"

echo "Запуск тестов с разными приоритетами..."
pids=()
for i in "${!EXECUTABLES[@]}"; do
    exe="${EXECUTABLES[$i]}"
    prio="${DIFFERENT_PRIORITIES[$i]}"
    echo "Запуск $exe с приоритетом $prio"
    sudo taskset -c 0 "$exe" "$prio" &
    pids+=($!)
done
wait_for_processes "${pids[@]}"


