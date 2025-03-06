#!/bin/bash

# Проверяем, что переданы как минимум два аргумента: папка и список файлов
if [ "$#" -lt 2 ]; then
    echo "Использование: $0 <папка> <файл1> <файл2> ... [--run <файл_для_запуска1> <файл_для_запуска2> ...]"
    exit 1
fi

# Первый аргумент — папка
folder=$1
shift

# Переходим в указанную папку
cd "$folder" || { echo "Не удалось перейти в папку $folder"; exit 1; }

# Массив для хранения имен скомпилированных файлов
compiled_files=()

# Компилируем каждый файл из списка
while [[ $# -gt 0 ]]; do
    if [[ "$1" == "--run" ]]; then
        shift  # Пропускаем флаг --run
        break  # Завершаем компиляцию, переходим к запуску
    fi
    if [ -f "$1" ]; then
        output_name="${1%.*}"  # Убираем расширение файла для имени выходного файла
        gcc -o "$output_name" "$1"
        if [ $? -eq 0 ]; then
            echo "Файл $1 успешно скомпилирован в $output_name"
            compiled_files+=("$output_name")  # Добавляем имя скомпилированного файла в массив
        else
            echo "Ошибка при компиляции файла $1"
        fi
    else
        echo "Файл $1 не найден в папке $folder"
    fi
    shift
done

# Проверяем, есть ли файлы для запуска
run_files=()
while [[ $# -gt 0 ]]; do
    run_files+=("$1")  # Добавляем файлы для запуска в массив
    shift
done

# Запускаем указанные файлы
for run_file in "${run_files[@]}"; do
    if [[ " ${compiled_files[@]} " =~ " ${run_file} " ]]; then
        echo "Запуск файла $run_file..."
        ./"$run_file"
        if [ $? -eq 0 ]; then
            echo "Файл $run_file успешно выполнен"
        else
            echo "Ошибка при выполнении файла $run_file"
        fi
    else
        echo "Файл $run_file не был скомпилирован или не найден"
    fi
done

# Удаляем все скомпилированные файлы
for compiled_file in "${compiled_files[@]}"; do
    if [ -f "$compiled_file" ]; then
        rm "$compiled_file"
        echo "Файл $compiled_file удалён"
    else
        echo "Файл $compiled_file не найден для удаления"
    fi
done