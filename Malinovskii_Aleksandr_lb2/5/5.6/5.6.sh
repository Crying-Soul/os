#!/bin/bash

cd 5/5.6

echo "вызов команды cat /proc/sys/kernel/sched_rt_runtime_us"
 cat /proc/sys/kernel/sched_rt_runtime_us
echo "/proc/sys/kernel/sched_rt_runtime_us определяет, сколько микросекунд (µs) процессорного времени выделяется задачам реального времени (RT) в каждом 1-секундном интервале (1000000 µs).
По умолчанию его значение — 950000, что означает, что задачи реального времени могут использовать до 950000 микросекунд (95%) CPU в секунду.
Оставшиеся 5% (50000 µs) зарезервированы для обычных (не RT) процессов, чтобы система оставалась отзывчивой."

files=("5.6.1.c" "5.6.2.c")
for file in "${files[@]}"
do
    output="${file%.*}"
    gcc "$file" -o "$output"
done

echo -e "\n5.6.1\n"
taskset -c 0 ./5.6.2 99 99 &
sleep 5
pkill 5.6.2 
 echo -e "\n5.6.2\n rr=40 fifo=30\n"
 taskset -c 0 ./5.6.2 30 40 &
 sleep 5
 pkill 5.6.2 
  echo -e "\n5.6.2\n rr=30 fifo=40\n"
 taskset -c 0 ./5.6.2 40 30 &
 sleep 5
pkill 5.6.2 
for file in "${files[@]}"
do
    output="${file%.*}"
    rm $output
done
