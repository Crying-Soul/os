#!/bin/bash

PROGRAM="./lb3/2"

run_test() {
    local FLAG=$1
    local SIGNAL=$2
    local COUNT=$3
    echo "Запуск: $FLAG"
    strace -tt -v -s 1000 -e trace=signal $PROGRAM $FLAG &
    PID=$!
    sleep 1  # Даем время запуститься
    
    for ((i=0; i<COUNT; i++)); do
        kill -$SIGNAL $PID
        echo "Отправлен сигнал $SIGNAL"
    done
    
    sleep 1
    kill $PID
    wait $PID 2>/dev/null
    echo "Завершен: $FLAG\n"
}

run_test "--process_single" SIGINT 1
run_test "--process_multi" SIGINT 3
run_test "--thread_single" SIGINT 1
run_test "--thread_multi" SIGINT 3
run_test "--custom_signal" SIGTSTP 1
