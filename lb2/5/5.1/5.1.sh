#!/bin/bash
"""
Функциональность:
1. Сравнительный анализ влияния nice-значений на планирование процессов:
   - Два тестовых прогона (одинаковые и разные nice-значения)
   - Привязка всех процессов к ядру 0 (taskset -c 0)
   - Статистика выполнения за фиксированный интервал (5 сек)

2. Управление тестами:
   - Автоматический запуск 4 процессов с разными параметрами
   - Сбор и анализ логов выполнения
   - Расчет процентного соотношения выполнения

3. Особенности реализации:
   - Параллельный запуск процессов в фоне
   - Учет PID для корректного завершения
   - Генерация отчетов в формате:
     Процесс <name> выполнился X раз (nice=N) - Y.YY%
"""
# Пути к исходным файлам и исполняемым программам
EXECUTABLES=("lb2/5/5.1/5.1-prog1" "lb2/5/5.1/5.1-prog2" "lb2/5/5.1/5.1-prog3" "lb2/5/5.1/5.1-prog4")

# Значения nice для второго запуска (разные)
DIFFERENT_NICE=(-15 -10 0 10)
# Значения nice для первого запуска (одинаковые)
SAME_NICE=(0 0 0 0)

# Создаем папку для трассировки
TRACE_DIR="trace"
mkdir -p "$TRACE_DIR"

# Функция для настройки и запуска трассировки lttng
start_tracing() {
    local test_label=$1
    local session_name="sched_trace_${test_label}"
    
    echo "Настройка трассировки для сессии: $session_name"
    
    # Удаляем предыдущую сессию если существует
    lttng destroy "$session_name" 2>/dev/null
    
    # Создаем новую сессию
    lttng create "$session_name" --output="$TRACE_DIR/$session_name"
    
    # Включаем события планировщика
    echo "Включение событий ядра..."
    lttng enable-event -k sched_switch,sched_process_fork,sched_process_exec,sched_process_exit
    lttng enable-event -k sched_waking,sched_wakeup,sched_migrate_task
    lttng enable-event -k sched_stat_runtime,sched_stat_sleep,sched_stat_iowait,sched_stat_blocked
    lttng enable-event -k sched_setparam
    
    # Запускаем трассировку
    lttng start
    echo "Трассировка начата"
}

# Функция для остановки трассировки
stop_tracing() {
    local test_label=$1
    local session_name="sched_trace_${test_label}"
    
    echo "Остановка трассировки для сессии: $session_name"
    lttng stop
    lttng destroy "$session_name"
    echo "Трассировка сохранена в $TRACE_DIR/$session_name"
}

# Функция для проведения теста с заданными значениями nice
run_test() {
    local -n NICE_ARRAY=$1   # Передача массива значений nice по ссылке
    local test_label=$2       # Метка для вывода и имен файлов с логами

    echo "========================================"
    echo "Запуск теста: $test_label"
    echo "========================================"

    # Очистка файлов с выводом (если они существуют)
    for executable in "${EXECUTABLES[@]}"; do
        local outfile="output_$(basename "$executable")_${test_label}.log"
        > "$outfile"
    done

    # Запуск трассировки
    start_tracing "$test_label"

    # Запуск процессов с указанными значениями nice и привязкой к ядру CPU (core 0)
    declare -a pids=()  # массив для хранения PID запущенных процессов
    for i in "${!EXECUTABLES[@]}"; do
        local exe="${EXECUTABLES[$i]}"
        local outfile="output_$(basename "$exe")_${test_label}.log"
        echo "Запуск процесса $exe с nice=${NICE_ARRAY[$i]}"
        taskset -c 0 nice -n "${NICE_ARRAY[$i]}" ./"$exe" >> "$outfile" 2>&1 &
        pids+=($!)   # сохраняем PID процесса
        echo "PID процесса: ${pids[-1]}"
    done

    # Пауза для выполнения процессов
    echo "Процессы работают в течение 5 секунд..."
    sleep 5

    # Остановка запущенных процессов по сохранённым PID
    echo "Завершение процессов..."
    for pid in "${pids[@]}"; do
        kill "$pid" 2>/dev/null
    done

    # Остановка трассировки
    stop_tracing "$test_label"

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


    echo "========================================"
    echo "Результаты теста: $test_label"
    echo "========================================"
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
    echo "----------------------------------------"
}

# Первый запуск: все процессы с одинаковым значением nice
run_test SAME_NICE "same_nice"

# Второй запуск: процессы с разными значениями nice
run_test DIFFERENT_NICE "diff_nice"

# Очистка временных файлов
for executable in "${EXECUTABLES[@]}"; do
    rm -f "output_$(basename "$executable")_same_nice.log"
    rm -f "output_$(basename "$executable")_diff_nice.log"
done

echo "========================================"
echo "Все тесты завершены"
echo "Трассировки сохранены в папку $TRACE_DIR"
echo "Для анализа трассировок используйте:"
echo "  babeltrace $TRACE_DIR/sched_trace_* | less"
echo "или откройте в Trace Compass"
echo "========================================"
