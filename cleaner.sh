#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Использование: $0 <директория_проекта>"
    exit 1
fi

project_dir="$1"


find "$project_dir" -type f -name "*.c" | while read c_file; do
    executable="${c_file%.c}"
    if [ -f "$executable" ]; then
        echo "Удаление $executable"
        rm -f "$executable"
    fi
done

echo "Все скомпилированные файлы удалены."SS