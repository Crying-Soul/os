#!/bin/bash

# =============================================================================
# Скрипт для бенчмаркинга и трассировки различных методов IPC
# =============================================================================
# Цель:
#   Сравнить производительность различных методов межпроцессного взаимодействия
#   (IPC) в Linux: POSIX shared memory, System V shared memory, и потоки POSIX.
#   Скрипт компилирует необходимые программы, запускает их с трассировкой
#   системных вызовов, и выполняет бенчмарки для измерения времени выполнения
#   и загрузки CPU.
#
# Описание:
#   Скрипт выполняет следующие действия:
#     1. Компилирует программы: posix.c, sys.c, threads.c и benchmark.c.
#     2. Трассирует программы posix, sys и threads с помощью strace для
#        анализа их системных вызовов.
#     3. Запускает бенчмарки с помощью программы benchmark для каждого метода
#        IPC (POSIX, System V, Pthreads).
#     4. Вычисляет среднее время выполнения и загрузку CPU для каждого метода.
#     5. Выводит результаты в табличном виде.
#
# Используемые методы IPC:
#   - POSIX Shared Memory:  Использование shm_open, mmap и семафоров POSIX.
#   - System V IPC: Использование shmget, shmat и семафоров System V.
#   - Pthreads: Использование потоков и мьютексов POSIX для обмена данными
#       внутри одного процесса.
#
# Параметры:
#   ITER: Количество итераций для обмена данными между процессами/потоками.
#   RUNS: Количество запусков бенчмарка для усреднения результатов.
#
# Зависимости:
#   - gcc
#   - strace
#   - bc (для вычислений с плавающей точкой)
#
# Вывод:
#   Скрипт выводит таблицу со средним временем выполнения и загрузкой CPU
#   для каждого метода IPC. Также выводит трассировку системных вызовов
#   с помощью strace.
#
# Примечания:
#   - Для запуска скрипта требуется root права, если используются вызовы,
#     требующие привилегий.
#   -  Значения ITER и RUNS можно изменить для получения более точных
#      результатов.
# =============================================================================

cd 5
gcc -o posix posix.c
gcc -o sys sys.c
gcc -o threads threads.c
gcc -o benchmark benchmark.c
# Пути к исполняемым файлам
POSIX_EXEC="./posix"
SYSV_EXEC="./sys"
PTHREAD_EXEC="./threads"
BENCHMARK_EXEC="./benchmark"

echo "=============================================="
echo "                  POSIX demonstration         "
echo "=============================================="
strace -f -e trace=read,write,futex -s 1000 ./posix 5 # -s 1000 — показывать больше данных
echo "=============================================="
echo "              SystemV_IPC demonstration        "
echo "=============================================="
strace -f -e trace=read,write,shmget,shmat,shmdt,semop,semctl,futex ./sys 5
echo "=============================================="
echo "                  Pthread  demonstration      "
echo "=============================================="
strace -f -e trace=read,write,futex ./threads 5 

# Количество итераций
ITER=10
# Количество запусков
RUNS=5

# Проверка исполняемого файла
check_exec() {
    if [ ! -x "$1" ]; then
        echo "Предупреждение: $1 не найден или не исполняемый. Пропускаем." >&2
        return 1
    fi
    return 0
}

# Запуск benchmark и вычисление средних значений
run_benchmark() {
    local name=$1
    shift

    if ! check_exec "$1"; then
        averages["$name,real"]="n/a"
        averages["$name,cpu"]="n/a"
        return
    fi

    local sum_real=0
    local sum_cpu=0
    local count=0
    local real_time cpu_load out

    for ((i=1; i<=RUNS; i++)); do
        out=$("$BENCHMARK_EXEC" "$1" "$ITER" 2>/dev/null)
        real_time=$(echo "$out" | awk '{print $1}')
        cpu_load=$(echo "$out" | awk '{print $2}')

        # Проверяем, что получили валидные числа
        if [[ ! "$real_time" =~ ^[0-9]+(\.[0-9]+)?$ || ! "$cpu_load" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
            echo "Ошибка в выводе benchmark ($out). Пропуск итерации." >&2
            continue
        fi

        sum_real=$(echo "$sum_real + $real_time" | bc -l)
        sum_cpu=$(echo "$sum_cpu + $cpu_load" | bc -l)
        ((count++))
    done

    if ((count > 0)); then
        local avg_real=$(echo "scale=9; $sum_real / $count" | bc -l)
        local avg_cpu=$(echo "scale=2; $sum_cpu / $count" | bc -l)
        averages["$name,real"]=$avg_real
        averages["$name,cpu"]=$avg_cpu
        echo "$name: avg real time = ${avg_real}s, avg CPU load = ${avg_cpu}%"
    else
        averages["$name,real"]="n/a"
        averages["$name,cpu"]="n/a"
    fi
}

declare -A averages

echo "-----------------------------------------------"
echo "Запуск бенчмарков (ITER=$ITER, RUNS=$RUNS)..."
echo "-----------------------------------------------"

run_benchmark "POSIX_IPC" "$POSIX_EXEC"
run_benchmark "SystemV_IPC" "$SYSV_EXEC"
run_benchmark "Pthread" "$PTHREAD_EXEC"

methods=("POSIX_IPC" "SystemV_IPC" "Pthread")

echo ""

printf "%22s %17s %15s\n" "Реализация" "Real Time (s)" "CPU Load (%)"
printf "%s %s %s\n" "--------------" "-----------------" "------------"

for name in "${methods[@]}"; do
    real_val=${averages["$name,real"]}
    cpu_val=${averages["$name,cpu"]}
    if [ -n "$real_val" ]; then
        if [ "$name" = "Pthread" ]; then
            echo -e "$name\t\t$real_val\t$cpu_val"  # Два \t после pthread
        else
            echo -e "$name\t$real_val\t$cpu_val"    # Один \t для остальных
        fi
    fi
done

rm benchmark posix sys threads