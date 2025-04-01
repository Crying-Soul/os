#!/bin/bash

# Компиляция всех программ
gcc tcp_server.c -o server_tcp -lpthread
gcc udp_server.c -o server_udp
gcc client.c -o client

run_test() {
    local protocol=$1
    local num_clients=$2
    
    echo -e "\n=== Testing $num_clients $protocol clients ==="
    
    # Создаем директорию для логов
    LOG_DIR="strace_logs"
    mkdir -p "$LOG_DIR"
    
    # Запуск сервера с strace (только сервер!)
    strace -f -o "$LOG_DIR/${protocol}_server_${num_clients}.strace" ./server_"$protocol" $num_clients &
    SERVER_PID=$!
    sleep 2  # Даем серверу время на запуск
    
    # Замер времени начала
    START_TIME=$(date +%s.%N)
    
    # Запуск клиентов (без strace)
    for i in $(seq 1 $num_clients); do
        ./client $protocol "CLIENT_$i" >/dev/null 2>&1 &
    done
    
    # Ожидание завершения всех клиентов
    wait
    
    
    # Замер времени окончания
    END_TIME=$(date +%s.%N)
    
    # Остановка сервера
    kill $SERVER_PID 2>/dev/null
    wait $SERVER_PID 2>/dev/null
    
    # Вычисление времени
    ELAPSED=$(echo "$END_TIME - $START_TIME" | bc)
    echo "Total time: $ELAPSED seconds"
    
    # Краткий анализ лога сервера
    echo -e "\nServer syscall summary:"
    grep -oP '^\d+\s+\K\w+' "$LOG_DIR/${protocol}_server_${num_clients}.strace" | sort | uniq -c | sort -nr | head -10
    echo ""
}

# Тестируемые количества клиентов
CLIENT_COUNTS=(10 100 1000)

# Очистка старых логов
rm -rf strace_logs

# Тестируем TCP
for count in "${CLIENT_COUNTS[@]}"; do
    run_test "tcp" $count
done

# Тестируем UDP
for count in "${CLIENT_COUNTS[@]}"; do
    run_test "udp" $count
done

# Удаление исполняемых файлов
rm client server_tcp server_udp
echo -e "\nAll tests completed"
echo "Server logs saved in strace_logs/"