#!/bin/bash

log() {
    local timestamp
    timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    echo -e "\n[$timestamp] $*"
}

log "sudo find / 2>/dev/null -type d -o -type f | awk -F/ '{ print NF-1, $0 }' | sort -n | tail -n 1"

deepest_path=$(sudo find / 2>/dev/null -type d -o -type f | awk -F/ '{ print NF-1, $0 }' | sort -n | tail -n 1)
echo "Самый глубокий путь: $deepest_path"

# Создаём папку с вложенностью 17 уровней
echo "Создание папки с вложенностью 17 уровней..."
path="folder"
for i in {1..17}; do
  path="$path/folder$i"
done
mkdir -p "$path"

# Поиск самого глубокого пути (по количеству вложенных директорий)
log "sudo find / 2>/dev/null -type d -o -type f | awk -F/ '{ print NF-1, $0 }' | sort -n | tail -n 1"
deepest_path=$(sudo find / 2>/dev/null -type d -o -type f | awk -F/ '{ print NF-1, $0 }' | sort -n | tail -n 1)
echo "Самый глубокий путь: $deepest_path"

# Поиск самого длинного пути (по количеству символов)
log "sudo find / 2>/dev/null -type d -o -type f | awk '{ print length, $0 }' | sort -n | tail -n 1"
longest_path=$(sudo find / 2>/dev/null -type d -o -type f | awk '{ print length, $0 }' | sort -n | tail -n 1)
echo "Самый длинный путь по количеству символов: $longest_path"


rm -rf folder1

