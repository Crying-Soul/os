#!/bin/bash

cd 5/5.1

# Компилируем программы
gcc -O0 1.c -o p1
gcc -O0 2.c -o p2
gcc -O0 3.c -o p3
gcc -O0 4.c -o p4
gcc -O0 5.c -o p5
gcc -O0 6.c -o p6

# Очищаем файлы вывода (если они существуют)
> output_p1.log
> output_p2.log
> output_p3.log
> output_p4.log
> output_p5.log
> output_p6.log

# Запускаем процессы с разными значениями nice и перенаправляем вывод в файлы
echo "Запуск процессов..."
taskset -c 0 nice -n -20 ./p1 >> output_p1.log 2>&1 & 
taskset -c 0 nice -n -10 ./p2 >> output_p2.log 2>&1 & 
taskset -c 0 nice -n 0 ./p3 >> output_p3.log 2>&1 &
taskset -c 0 nice -n 10 ./p4 >> output_p4.log 2>&1 &
taskset -c 0 nice -n -5 ./p5 >> output_p5.log 2>&1 &
taskset -c 0 nice -n 5 ./p6 >> output_p6.log 2>&1 &

echo "Процессы запущены."

# Даем процессам время для выполнения
echo "Ожидание выполнения процессов (5 секунд)..."
sleep 5
echo "Время выполнения истекло."

# Останавливаем процессы
echo "Остановка процессов..."
pkill p1
pkill p2
pkill p3
pkill p4
pkill p5
pkill p6
echo "Процессы остановлены."

# Подсчитываем количество выполнений каждого процесса
echo "Подсчет количества выполнений..."
count_p1=$(wc -l < output_p1.log)
count_p2=$(wc -l < output_p2.log)
count_p3=$(wc -l < output_p3.log)
count_p4=$(wc -l < output_p4.log)
count_p5=$(wc -l < output_p5.log)
count_p6=$(wc -l < output_p6.log)


# Рассчитываем общее количество выполнений
total_count=$((count_p1 + count_p2 + count_p3 + count_p4 + count_p5 + count_p6))

# Рассчитываем процент выполнения каждого процесса с точным округлением
percent_p1=$(echo "scale=4; $count_p1 / $total_count * 100" | bc)
percent_p2=$(echo "scale=4; $count_p2 / $total_count * 100" | bc)
percent_p3=$(echo "scale=4; $count_p3 / $total_count * 100" | bc)
percent_p4=$(echo "scale=4; $count_p4 / $total_count * 100" | bc)
percent_p5=$(echo "scale=4; $count_p5 / $total_count * 100" | bc)
percent_p6=$(echo "scale=4; $count_p6 / $total_count * 100" | bc)

# Выводим подробные результаты
echo "=============================================="
echo "Результаты выполнения процессов:"
echo "----------------------------------------------"
echo "Процесс p1 (nice=-20):"
echo "  Количество выполнений: $count_p1"
echo "  Процент выполнения: $percent_p1%"
echo "----------------------------------------------"
echo "Процесс p2 (nice=-10):"
echo "  Количество выполнений: $count_p2"
echo "  Процент выполнения: $percent_p2%"
echo "----------------------------------------------"
echo "Процесс p3 (nice=0):"
echo "  Количество выполнений: $count_p3"
echo "  Процент выполнения: $percent_p3%"
echo "----------------------------------------------"
echo "Процесс p4 (nice=10):"
echo "  Количество выполнений: $count_p4"
echo "  Процент выполнения: $percent_p4%"
echo "----------------------------------------------"
echo "Процесс p5 (nice=-5):"
echo "  Количество выполнений: $count_p5"
echo "  Процент выполнения: $percent_p5%"
echo "----------------------------------------------"
echo "Процесс p6 (nice=5):"
echo "  Количество выполнений: $count_p6"
echo "  Процент выполнения: $percent_p6%"
echo "----------------------------------------------"
echo "Общее количество выполнений всех процессов: $total_count"
echo "=============================================="

# Удаляем временные файлы
echo "Очистка временных файлов..."
rm output_*
rm p*
echo "Временные файлы удалены."