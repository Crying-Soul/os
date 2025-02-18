#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: $0 <output_file>"
    exit 1
fi

output_file="$1"
> "$output_file"

find_type() {
    local type="$1"
    local found=0
    sudo ls -lR / 2>/dev/null | awk -v type="$type" -v found="$found" '
    /:$/ {
        sub(/:$/, "", $0)
        dir = $0
        next
    }
    $1 ~ "^"type {
        if (dir == "/")
            printf "%s %s\n", $1, dir $NF;
        else
            printf "%s %s/%s\n", $1, dir, $NF;
        exit
    }
' >> "$output_file"

}

echo "Scanning..."

find_type "-"  # обычные файлы
find_type "b"  # блочные устройства
find_type "c"  # символьные устройства
find_type "d"  # директории
find_type "l"  # символические ссылки
find_type "p"  # именованные каналы (FIFO)
find_type "s"  # сокеты

echo "Results written in $output_file"
