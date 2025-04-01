#!/bin/bash

# Компиляция всех программ
gcc tcp_server.c -o server_tcp -lpthread
gcc udp_server.c -o server_udp
gcc client.c -o client

run_test() {
    local protocol=$1
    local num_clients=$2
    
    echo -e "\n=== Testing $num_clients $protocol clients ==="
    
    # Запуск сервера с исправленным аргументом
    ./server_"$1" $num_clients &
    SERVER_PID=$!

    # Замер времени начала
    START_TIME=$(date +%s.%N)
    
    # Запуск клиентов
    for i in $(seq 1 $num_clients); do
        ./client $protocol "CLIENT_$i" >/dev/null 2>&1 &
    done
    
    # Ожидание завершения сервера
    wait $SERVER_PID 2>/dev/null
    
    # Замер времени окончания
    END_TIME=$(date +%s.%N)
    
    # Вычисление времени
    ELAPSED=$(echo "$END_TIME - $START_TIME" | bc)
    echo "Total time: $ELAPSED seconds"
}


# Тестируемые количества клиентов
CLIENT_COUNTS=(10 100 1000)

 # Тестируем TCP
 for count in "${CLIENT_COUNTS[@]}"; do
    run_test "tcp" $count
 done

# Тестируем UDP
for count in "${CLIENT_COUNTS[@]}"; do
    run_test "udp" $count
done

rm client server_tcp server_udp
echo -e "\nAll tests completed"