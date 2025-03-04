#!/bin/bash

# Папка с файлами
cd exec_examples

# Массив с именами исходных файлов
files=("execl_example.c" "execlp_example.c" "execle_example.c" "execv_example.c" "execvp_example.c" "execve_example.c")

# Компиляция и запуск каждой программы
for file in "${files[@]}"
do
    # Извлекаем имя файла без расширения
    output="${file%.*}"
    
    # Компилируем программу
    gcc "$file" -o "$output"

    # Запускаем скомпилированную программу
   
    ./$output
    echo "------------------------"
    rm $output
done
