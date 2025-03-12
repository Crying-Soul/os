#!/bin/bash

# Проверяем, что передана папка
if [ -z "$1" ]; then
  echo "Использование: $0 <папка>"
  exit 1
fi

# Функция для рекурсивной компиляции .c файлов
compile_c_files() {
  local dir=$1

  # Рекурсивно ищем .c файлы и компилируем их
  find "$dir" -name "*.c" | while read -r file; do
    output_file="${file%.c}"  # Убираем расширение .c
    echo "Компиляция $file в $output_file"
    gcc -O0 "$file" -o "$output_file"
  done
}

# Вызываем функцию с переданной папкой
compile_c_files "$1"