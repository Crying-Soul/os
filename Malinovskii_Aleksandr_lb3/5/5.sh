#!/bin/bash

# Параметры тестирования
ITERATIONS=1000
RUNS=5
MESSAGE_SIZE=1024

# Исходные файлы
POSIX_SRC="posix.c"
SYSV_SRC="sys.c"
THREADS_SRC="threads.c"

# Исполняемые файлы
POSIX_BIN="posix"
SYSV_BIN="sys"
THREADS_BIN="threads"

compile_programs() {
    echo "Компиляция программ..."
    gcc -O2 -o $POSIX_BIN $POSIX_SRC -lrt -lpthread || { echo "Ошибка компиляции $POSIX_SRC"; exit 1; }
    gcc -O2 -o $SYSV_BIN $SYSV_SRC || { echo "Ошибка компиляции $SYSV_SRC"; exit 1; }
    gcc -O2 -o $THREADS_BIN $THREADS_SRC -lpthread || { echo "Ошибка компиляции $THREADS_SRC"; exit 1; }
    echo "Компиляция завершена успешно"
}

measure_time() {
    local program=$1
    local iters=$2
    local runs=$3
    local total_time=0
    declare -a run_times
    
    echo "Тестирование $program (итераций: $iters):"
    
    for ((i=1; i<=runs; i++)); do
        # Используем date с наносекундной точностью
        start_time=$(date +%s.%N)
        ./$program $iters >/dev/null 2>&1
        end_time=$(date +%s.%N)
        
        # Вычисляем время выполнения с проверкой ошибок
        runtime=$(awk -v start="$start_time" -v end="$end_time" 'BEGIN {printf "%.4f", end - start}')
        if [[ "$runtime" == "0.0000" || -z "$runtime" ]]; then
            runtime="0.0001"  # Минимальное значение чтобы избежать nan
        fi
        
        run_times[$i]=$runtime
        total_time=$(awk -v total="$total_time" -v rt="$runtime" 'BEGIN {printf "%.4f", total + rt}')
        printf "  Прогон %d: %s сек\n" $i "$runtime"
    done
    
    average_time=$(awk -v total="$total_time" -v runs="$runs" 'BEGIN {printf "%.4f", total/runs}')
    echo "---------------------------------"
    printf "  Среднее время: %s сек\n" "$average_time"
    echo
}

cleanup() {
    echo "Очистка ресурсов..."
    rm -f /dev/shm/my_shm 2>/dev/null
    ipcrm -a 2>/dev/null || echo "Для очистки System V ресурсов可能需要 права root"
    rm -f $POSIX_BIN $SYSV_BIN $THREADS_BIN 2>/dev/null
}

echo "=== Тестирование производительности ==="
echo "Параметры:"
echo "  Итераций: $ITERATIONS"
echo "  Прогонов: $RUNS"
echo "  Размер сообщения: $MESSAGE_SIZE байт"
echo

compile_programs
echo

measure_time $POSIX_BIN $ITERATIONS $RUNS
measure_time $SYSV_BIN $ITERATIONS $RUNS
measure_time $THREADS_BIN $ITERATIONS $RUNS

cleanup
echo "Тестирование завершено"
