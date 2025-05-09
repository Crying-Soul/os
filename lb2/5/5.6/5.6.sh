#!/bin/bash
"""
Функциональность:
1. Комплексное тестирование работы потоков с разными параметрами:
   - Три сценария с разными комбинациями приоритетов
   - Принудительная привязка к CPU 0 (taskset -c 0)
   - Контролируемое время выполнения (15 сек на тест)
   - Автоматическое завершение процессов

2. Тестовые сценарии:
   - Одинаковые приоритеты (RR=50, FIFO=50)
   - Высокий приоритет FIFO (RR=50, FIFO=10)
   - Высокий приоритет RR (RR=10, FIFO=50)
"""
# Пути к исполняемым файлам
EXECUTABLES=("src/5/5.6/5.6")



echo "Запуск потоков с одинаковыми приоритетами (RR=50 FIFO=50)..."
pids=()
for exe in "${EXECUTABLES[@]}"; do
    sudo taskset -c 0 "$exe" 50 50 &
    pids+=($!)
done

sleep 15

for pid in "${pids[@]}"; do
    kill "$pid" 2>/dev/null
done

echo -e "\nЗапуск потоков с разными приоритетами (RR=50 FIFO=10)..."
pids=()
for exe in "${EXECUTABLES[@]}"; do
    sudo taskset -c 0 "$exe" 10 50 &
    pids+=($!)
done

sleep 15

for pid in "${pids[@]}"; do
    kill "$pid" 2>/dev/null
done


echo -e "\nЗапуск потоков с разными приоритетами (RR=10 FIFO=50)..."
pids=()
for exe in "${EXECUTABLES[@]}"; do
    sudo taskset -c 0 "$exe" 50 10 &
    pids+=($!)
done

sleep 15

for pid in "${pids[@]}"; do
    kill "$pid" 2>/dev/null
done





