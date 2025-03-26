#!/bin/bash
"""
Функциональность:
1. Тестирование FIFO-планирования с одинаковыми приоритетами:
   - Запуск 3 процессов с priority=30
   - Принудительная привязка к CPU 0 (taskset -c 0)

2. Особенности:
   - Использование sudo для установки реального времени
   - Параллельный запуск процессов
   - Ожидание завершения всех процессов
   - Чистый вывод состояния в консоль
"""
# Пути к исполняемым файлам
EXECUTABLES=("src/5/5.3/5.3-prog1" "src/5/5.3/5.3-prog2" "src/5/5.3/5.3-prog3")

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




