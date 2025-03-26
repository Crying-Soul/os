#!/bin/bash
# Улучшенный скрипт бенчмарка с точным выводом чисел.

# Пути к исполняемым файлам
POSIX_EXEC="./lb3/5/5-posix"
SYSV_EXEC="./lb3/5/5-systemv"
PTHREAD_EXEC="./lb3/5/5-pthread"

BENCHMARK_EXEC="./lb3/5/5-benchmark"

# Количество итераций
ITER=10000
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
        printf "%12s %17.9f %10.2f\n" "$name" "$real_val" "$cpu_val"
    fi
done
