#!/bin/bash

# Конфигурация
SERVER_IP="127.0.0.1"
LOG_DIR="strace_logs"
CLIENT_COUNTS=(10 100 1000)
SERVER_PATH="./lb3/7/7.3/server"
CLIENT_PATH="./lb3/7/7.3/client"
SERVER_START_DELAY=2
MAX_WAIT_TIME=30
ANALYSIS_ONLY=false # Режим только для анализа (без запуска тестов)

# Функция очистки старых логов и процессов
cleanup() {
    if ! $ANALYSIS_ONLY; then
        echo "Очистка старых логов и процессов..."
        rm -rf "$LOG_DIR"
        mkdir -p "$LOG_DIR"
        pkill -f "$SERVER_PATH" 2>/dev/null
        pkill -f "$CLIENT_PATH" 2>/dev/null
        sleep 1
    fi
}

# Функция анализа логов
analyze_logs() {
    local protocol=$1
    local num_clients=$2

    echo -e "\n[Анализ для $num_clients $protocol клиентов]"

    # Анализ системных вызовов сервера
    local server_trace="$LOG_DIR/${protocol}_server_${num_clients}.strace"
    if [[ -f "$server_trace" ]]; then
        echo -e "\nТоп-10 системных вызовов сервера:"
        grep -oE '^[0-9]+\s+([a-zA-Z0-9_]+)' "$server_trace" | \
            awk '{print $2}' | sort | uniq -c | sort -nr | head -10
    else
        echo "Лог strace сервера не найден: $server_trace"
    fi

    # Анализ ошибок в логах клиентов
    local client_logs=("$LOG_DIR/${protocol}_client_"*.log)
    if [ ${#client_logs[@]} -gt 0 ] && [ -f "${client_logs[0]}" ]; then
        # Сохраняем файлы, содержащие "ERROR", в массив error_files
        local error_files=()
        while IFS= read -r line; do
            error_files+=("$line")
        done < <(grep -l "ERROR" "$LOG_DIR/${protocol}_client_"*.log 2>/dev/null)
        
        local error_count=${#error_files[@]}

        if [[ $error_count -gt 0 ]]; then
            echo -e "\nНайдены ошибки в $error_count лог(ах) клиентов:"
            for file in "${error_files[@]:0:3}"; do
                # Выводим первую найденную ошибку из файла
                echo "  ${file##*/}: $(grep "ERROR" "$file" | head -1)"
            done
            if [[ $error_count -gt 3 ]]; then
                echo "  ...и ещё $((error_count - 3))"
            fi
        else
            echo -e "\nОшибок в логах клиентов не обнаружено"
        fi
    else
        echo "Логи клиентов не найдены для протокола $protocol с $num_clients клиентами."
    fi

    # Проверка незавершённых клиентов
    local incomplete
    incomplete=$(grep -l "Did not complete" "$LOG_DIR/${protocol}_client_"*.log 2>/dev/null | wc -l)
    if [[ $incomplete -gt 0 ]]; then
        echo -e "\nВнимание: $incomplete клиентов не завершили работу успешно"
    fi
}

# Функция для запуска одного теста
run_test() {
    local protocol=$1
    local num_clients=$2

    echo -e "\n=== Тестирование $num_clients $protocol клиентов ==="
    
    # Запуск сервера с отслеживанием системных вызовов
    strace -f -o "$LOG_DIR/${protocol}_server_${num_clients}.strace" \
           "$SERVER_PATH" --"$protocol" "$num_clients" >/dev/null 2>&1 &
    SERVER_PID=$!

    sleep "$SERVER_START_DELAY"

    if ! ps -p "$SERVER_PID" >/dev/null; then
        echo "Ошибка: сервер не запустился!"
        return 1
    fi

    # Запуск клиентов
    CLIENT_PIDS=()  # Инициализация массива
    START_TIME=$(date +%s.%N)
    for i in $(seq 1 "$num_clients"); do
        "$CLIENT_PATH" --"$protocol" --server "$SERVER_IP" "CLIENT_$i" \
            >"$LOG_DIR/${protocol}_client_${i}.log" 2>&1 &
        CLIENT_PIDS+=($!)
    done

    # Ожидание завершения сервера
    wait "$SERVER_PID" 2>/dev/null

    # Ожидание завершения клиентов с таймаутом
    local wait_time=0
    while [[ $wait_time -lt $MAX_WAIT_TIME ]]; do
        local running=0
        for pid in "${CLIENT_PIDS[@]}"; do
            if ps -p "$pid" >/dev/null; then
                ((running++))
            fi
        done

        [[ $running -eq 0 ]] && break
        ((wait_time++))
        sleep 1
    done

    if [[ $wait_time -ge $MAX_WAIT_TIME ]]; then
        echo "Предупреждение: не все клиенты завершились в течение $MAX_WAIT_TIME секунд"
        pkill -P $$ 2>/dev/null
    fi

    END_TIME=$(date +%s.%N)
    ELAPSED=$(printf "%.2f" "$(echo "$END_TIME - $START_TIME" | bc)")
    echo "Общее время выполнения: $ELAPSED секунд"

    # Выполняем анализ логов для каждого запуска
    analyze_logs "$protocol" "$num_clients"
}

main() {
    cleanup

    # Проверка наличия исполняемых файлов
    for prog in "$SERVER_PATH" "$CLIENT_PATH"; do
        if [[ ! -x "$prog" ]]; then
            echo "Ошибка: Файл не найден или не имеет прав на исполнение: $prog"
            exit 1
        fi
    done

    # Запуск тестов
    if ! $ANALYSIS_ONLY; then
        for count in "${CLIENT_COUNTS[@]}"; do
            run_test "tcp" "$count"
            cleanup

            run_test "udp" "$count"
            cleanup
        done
    fi
}

# Обработка аргументов командной строки
while getopts "a" opt; do
    case $opt in
        a) ANALYSIS_ONLY=true ;;
        *) 
           echo "Использование: $0 [-a] (режим только анализа)"
           exit 1
           ;;
    esac
done

main
