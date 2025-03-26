#!/bin/bash

POSIX_EXEC="./lb3/5/5-posix"
SYSV_EXEC="./lb3/5/5-systemv"
THREADS_EXEC="./lb3/5/5-threads"
ITERATIONS=1000
RUNS=5

echo "Benchmarking..."

get_cpu_usage() {
    local pid=$1
    local cpu_usage=0
    local count=0
    
    while kill -0 $pid 2>/dev/null; do
        current_cpu=$(ps -p $pid -o %cpu 2>/dev/null | tail -n 1 | awk '{print $1}')
        # Проверяем, что current_cpu - это число
        if [[ $current_cpu =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
            cpu_usage=$(echo "$cpu_usage + $current_cpu" | bc 2>/dev/null || echo "$cpu_usage")
            count=$((count + 1))
        fi
        sleep 0.1
    done
    
    if [ $count -gt 0 ]; then
        echo "scale=2; $cpu_usage / $count" | bc 2>/dev/null || echo "0"
    else
        echo "0"
    fi
}

run_benchmark() {
    local name=$1
    local exec=$2
    local total_time=0
    local total_cpu=0
    
    echo "=== $name ==="
    for ((i=1; i<=$RUNS; i++)); do
        start_time=$(date +%s.%N)
        $exec $ITERATIONS > /dev/null &
        pid=$!
        cpu=$(get_cpu_usage $pid)
        wait $pid
        end_time=$(date +%s.%N)
        elapsed=$(echo "$end_time - $start_time" | bc)
        
        total_time=$(echo "$total_time + $elapsed" | bc)
        total_cpu=$(echo "$total_cpu + $cpu" | bc)
        echo "Run $i: Time: $(printf "%.4f" $elapsed) sec, CPU: $(printf "%.2f" $cpu)%"
    done
    
    avg_time=$(echo "scale=4; $total_time / $RUNS" | bc)
    avg_cpu=$(echo "scale=2; $total_cpu / $RUNS" | bc)
    echo "Average: Time: $(printf "%.4f" $avg_time) sec, CPU: $(printf "%.2f" $avg_cpu)%"
    echo ""
    
    # Возвращаем результаты через глобальные переменные
    BENCH_TIME=$avg_time
    BENCH_CPU=$avg_cpu
}

# Запускаем тесты
run_benchmark "POSIX shared memory + semaphores" "$POSIX_EXEC"
POSIX_TIME=$BENCH_TIME
POSIX_CPU=$BENCH_CPU

run_benchmark "System V shared memory + semaphores" "$SYSV_EXEC"
SYSV_TIME=$BENCH_TIME
SYSV_CPU=$BENCH_CPU

run_benchmark "Pthreads mutex + condition variables" "$THREADS_EXEC"
PTHREADS_TIME=$BENCH_TIME
PTHREADS_CPU=$BENCH_CPU

# Выводим итоговые результаты
echo "=== Final Results ==="
echo "1. POSIX shared memory + semaphores:"
echo "   Time: $(printf "%.4f" $POSIX_TIME) sec, CPU: $(printf "%.2f" $POSIX_CPU)%"
echo "2. System V shared memory + semaphores:"
echo "   Time: $(printf "%.4f" $SYSV_TIME) sec, CPU: $(printf "%.2f" $SYSV_CPU)%"
echo "3. Pthreads mutex + condition variables:"
echo "   Time: $(printf "%.4f" $PTHREADS_TIME) sec, CPU: $(printf "%.2f" $PTHREADS_CPU)%"