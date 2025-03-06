#!/bin/bash
set -euo pipefail

# Функция для очистки фоновых процессов при завершении скрипта
cleanup() {
  echo "Очистка фоновых процессов..."
  for pid in "${job_pids[@]:-}"; do
    if kill -0 "$pid" 2>/dev/null; then
      echo "Останавливаем процесс с PID $pid"
      kill -9 "$pid" 2>/dev/null || true  # Жесткий сигнал для завершения
    fi
  done
}

# Трап для вызова cleanup при завершении скрипта или получении сигнала
trap cleanup EXIT

echo "Запуск фоновых процессов..."

# Массив для хранения PID запущенных процессов
job_pids=()

# Задание 1: имитация работы (sleep 1)
sleep 1 &
job1_pid=$!
job_pids+=("$job1_pid")
echo "Задание 1 (sleep 1) запущено с PID $job1_pid"

# Задание 2: непрерывное преобразование текста в верхний регистр (yes > /dev/null)
yes > /dev/null &
job2_pid=$!
job_pids+=("$job2_pid")
echo "Задание 2 (yes > /dev/null) запущено с PID $job2_pid"

# Задание 3: имитация длительной работы (sleep 4)
sleep 4 &
job3_pid=$!
job_pids+=("$job3_pid")
echo "Задание 3 (sleep 4) запущено с PID $job3_pid"

echo -e "\nАнализ списка заданий:"
jobs -l

# Прерывание задания 2 (если оно еще выполняется)
echo -e "\nПрерывание задания 2..."
if kill -0 "$job2_pid" 2>/dev/null; then
    kill -9 "$job2_pid" 2>/dev/null && echo "Задание 2 было прервано." || echo "Не удалось прервать задание 2."
else
    echo "Задание 2 уже завершилось или его нельзя прервать."
fi
sleep 1

echo -e "\nАнализ списка заданий после прерывания задания 2:"
jobs -l

# Ожидание завершения задания 1
echo -e "\nОжидание завершения задания 1..."
if wait "$job1_pid"; then
    echo "Задание 1 успешно завершено."
else
    echo "Задание 1 завершилось с ошибкой."
fi


echo -e "\nИтоговый анализ списка заданий:"
jobs -l

echo -e "\nСкрипт завершен."
