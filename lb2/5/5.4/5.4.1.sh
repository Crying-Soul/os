#!/bin/bash

# Пути к исполняемым файлам
EXECUTABLES=("lb2/5/5.4/5.4-prog1" "lb2/5/5.4/5.4-prog2" "lb2/5/5.4/5.4-prog3")

# Приоритет для запуска с одинаковыми приоритетами
SAME_PRIORITY=30


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




