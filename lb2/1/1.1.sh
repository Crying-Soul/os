#!/bin/bash
set -euo pipefail

# Функция для очистки фоновых процессов при завершении скрипта
cleanup() {
  echo "Очистка фоновых процессов..."
  for pid in "${job_pids[@]:-}"; do
    if kill -0 "$pid" 2>/dev/null; then
      echo "Останавливаем процесс с PID $pid"
      kill -9 "$pid" 2>/dev/null || true # Жесткий сигнал для завершения
    fi
  done
}

touch largefile.txt
# Трап для вызова cleanup при завершении скрипта или получении сигнала
trap cleanup EXIT

echo "Запуск фоновых процессов..."

# Массив для хранения PID запущенных процессов
job_pids=()

# Задание 1: Поиск файлов в директории (find)
echo "Задание 1: Поиск всех .txt файлов в /tmp..."
find /tmp -name "*.txt" >found_files.txt &
job1_pid=$!
job_pids+=("$job1_pid")
echo "Задание 1 (find) запущено с PID $job1_pid"

# Задание 2: Подсчет строк в файле (wc)
echo "Задание 2: Подсчет строк в файле largefile.txt..."
wc -l largefile.txt >line_count.txt &
job2_pid=$!
job_pids+=("$job2_pid")
echo "Задание 2 (wc) запущено с PID $job2_pid"

# Задание 3: Копирование файла (cp)
echo "Задание 3: Копирование файла largefile.txt в /tmp..."
cp largefile.txt /tmp/largefile_copy.txt &
job3_pid=$!
job_pids+=("$job3_pid")
echo "Задание 3 (cp) запущено с PID $job3_pid"

echo -e "\nАнализ списка заданий:"
jobs -l

# Прерывание задания 2 (wc), если оно еще выполняется
echo -e "\nПрерывание задания 2 (wc)..."
if kill -0 "$job2_pid" 2>/dev/null; then
  kill -9 "$job2_pid" 2>/dev/null && echo "Задание 2 (wc) было прервано." || echo "Не удалось прервать задание 2."
else
  echo "Задание 2 (wc) уже завершилось или его нельзя прервать."
fi
sleep 1 # Даем время для обновления статуса

echo -e "\nАнализ списка заданий после прерывания задания 2 (wc):"
jobs -l

# Ожидание завершения задания 1 (find)
echo -e "\nОжидание завершения задания 1 (find)..."
if wait "$job1_pid"; then
  echo "Задание 1 (find) успешно завершено."
else
  echo "Задание 1 (find) завершилось с ошибкой."
fi

echo -e "\nИтоговый анализ списка заданий:"
jobs -l

echo -e "\nСкрипт завершен."
rm found_files.txt line_count.txt largefile.txt
