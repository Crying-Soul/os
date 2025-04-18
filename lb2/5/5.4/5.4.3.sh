#!/bin/bash
"""
Функциональность:
1. Тестирование RR-планирования с разными приоритетами:
   - Процесс 1: priority=20 (низкий)
   - Процесс 2: priority=99 (максимальный)
   - Процесс 3: priority=99 (максимальный)
   - Процесс 4: priority=40 (средний)
   - Демонстрация вытеснения низкоприоритетных процессов

2. Особенности:
   - Явное задание разных приоритетов через массив
   - Визуализация строгого соблюдения приоритетов в RR
   - Автоматическое завершение теста
"""
# Пути к исполняемым файлам
EXECUTABLES=("src/5/5.4/5.4-prog1" "src/5/5.4/5.4-prog2" "src/5/5.4/5.4-prog3" "src/5/5.4/5.4-prog4")



# Приоритеты для запуска с разными приоритетами (для каждого из 3-х процессов)
DIFFERENT_PRIORITIES=(20 99 99 40)

# Функция для ожидания завершения группы процессов
wait_for_processes() {
    for pid in "$@"; do
        wait "$pid"
    done
}


echo "Запуск тестов с разными приоритетами..."
pids=()
for i in "${!EXECUTABLES[@]}"; do
    exe="${EXECUTABLES[$i]}"
    prio="${DIFFERENT_PRIORITIES[$i]}"
    echo "Запуск $exe с приоритетом $prio"
    sudo taskset -c 0 "$exe" "$prio" &
    pids+=($!)
done

sleep 15

for pid in "${pids[@]}"; do
    kill "$pid" 2>/dev/null
done




