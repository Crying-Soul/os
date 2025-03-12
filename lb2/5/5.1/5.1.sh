#!/bin/bash

# Пути к исходным файлам и исполняемым программам
EXECUTABLES=("lb2/5/5.1/5.1-prog1" "lb2/5/5.1/5.1-prog2" "lb2/5/5.1/5.1-prog3" "lb2/5/5.1/5.1-prog4")

# Значения nice для второго запуска (разные)
DIFFERENT_NICE=(-15 -10 0 10)
# Значения nice для первого запуска (одинаковые)
SAME_NICE=(0 0 0 0)

# Функция для проведения теста с заданными значениями nice
run_test() {
    local -n NICE_ARRAY=$1   # Передача массива значений nice по ссылке
    local test_label=$2        # Метка для вывода и имен файлов с логами

    echo "Запуск теста: $test_label"

    # Очистка файлов с выводом (если они существуют)
    for executable in "${EXECUTABLES[@]}"; do
        local outfile="output_$(basename "$executable")_${test_label}.log"
        > "$outfile"
    done

    # Запуск процессов с указанными значениями nice и привязкой к ядру CPU (core 0)
    declare -a pids=()  # массив для хранения PID запущенных процессов
    for i in "${!EXECUTABLES[@]}"; do
        local exe="${EXECUTABLES[$i]}"
        local outfile="output_$(basename "$exe")_${test_label}.log"
        taskset -c 0 nice -n "${NICE_ARRAY[$i]}" ./"$exe" >> "$outfile" 2>&1 &
        pids+=($!)   # сохраняем PID процесса
    done

    # Пауза для выполнения процессов
    sleep 5

    # Остановка запущенных процессов по сохранённым PID
    for pid in "${pids[@]}"; do
        kill "$pid" 2>/dev/null
    done

    # Подсчёт количества выполнений каждого процесса
    declare -A counts
    for executable in "${EXECUTABLES[@]}"; do
        local outfile="output_$(basename "$executable")_${test_label}.log"
        counts["$executable"]=$(wc -l < "$outfile")
    done

    # Подсчёт общего количества выполнений
    local total_count=0
    for count in "${counts[@]}"; do
        ((total_count += count))
    done

    echo "Результаты теста: $test_label"
    # Если общий счёт равен 0, чтобы избежать деления на ноль, установим его в 1
    if [ "$total_count" -eq 0 ]; then
        total_count=1
    fi

    # Вывод количества выполнений и процента для каждого процесса
    for i in "${!EXECUTABLES[@]}"; do
        local exe="${EXECUTABLES[$i]}"
        local count="${counts[$exe]}"
        local percent=$(echo "scale=4; $count / $total_count * 100" | bc)
        percent=$(printf "%.2f" "$percent")
        echo "Процесс $exe выполнился $count раз (nice=${NICE_ARRAY[$i]}) - $percent%"
    done
    echo "----------------------------"
}

# Первый запуск: все процессы с одинаковым значением nice
run_test SAME_NICE "Одинаковые значения nice"

# Второй запуск: процессы с разными значениями nice
run_test DIFFERENT_NICE "Разные значения nice"



for executable in "${EXECUTABLES[@]}"; do
    rm -f "output_$(basename "$executable")_Одинаковые значения nice.log"
    rm -f "output_$(basename "$executable")_Разные значения nice.log"
done