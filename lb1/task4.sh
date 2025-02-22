#!/bin/bash

# Перенаправляем весь вывод в файл out.txt
exec > out.txt 2>&1

# Проверяем, был ли передан параметр (путь к файлу)
if [ -z "$1" ]; then
  echo "Укажите путь к файлу."
  exit 1
fi

target_file="$1"

# Вариант 1: Использование ls с фильтрацией через grep
echo "Поиск с использованием ls и grep:"
ls -lRa /home/lotos | grep " -> " | grep "$target_file"
echo "----------------------------"

# Вариант 2: Использование find и stat
echo "Поиск с использованием find и stat:"
find /home/lotos -type l -exec stat -c "%N" {} \; 2>/dev/null | grep "$target_file"
echo "----------------------------"

echo "Поиск с использованием find и -lname:"
find /home/lotos -type l -lname "*$target_file*" 2>/dev/null
echo "----------------------------"

echo "Поиск с использованием readlink в цикле:"
for link in $(find /home/lotos -type l 2>/dev/null); do
  if readlink "$link" | grep -q "$target_file"; then
    echo "$link -> $(readlink "$link")"
  fi
done
