#!/bin/bash

# Параметры
num_files=10        # Количество файлов для создания
file_size=1M        # Размер каждого файла
num_delete=5        # Количество файлов для удаления

# Создание новой директории
mkdir test_dir
cd test_dir

echo "Размер добавляемого файла ${file_size}"

# Функция для измерения размера директории
get_dir_size() {
    du -sb . | awk '{print $1}'  # Получить размер в байтах
}

# Измерение начального размера директории
initial_size=$(get_dir_size)
echo "Начальный размер директории: $initial_size байт"

# Создание файлов
for i in $(seq 1 $num_files); do
    dd if=/dev/zero of=file$i bs=$file_size count=1 > /dev/null 2>&1  # Создать файл указанного размера
    current_size=$(get_dir_size)
    echo "Добавлен file$i. Размер директории: $current_size байт"
done

# Удаление некоторых файлов
for i in $(seq 1 $num_delete); do
    rm file$i  # Удалить файл
    current_size=$(get_dir_size)
    echo "Удален file$i. Размер директории: $current_size байт"
done

# Очистка: удаление директории
cd ..
rm -rf test_dir