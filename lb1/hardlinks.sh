#!/bin/bash

if [ $# -ne 1 ]; then
  echo "Usage: $0 /path/to/file"
  exit 1
fi

target="$1"
output_file="hard_links.txt"
> "$output_file"

if [ ! -e "$target" ]; then
  echo "Error: File '$target' not found!"
  exit 1
fi

# Получаем inode и устройство файла
inode=$(stat -c '%i' "$target")
device=$(stat -c '%d' "$target")

echo "Searching for hard links to '$target' (inode: $inode, device: $device)..."

# Используем альтернативный способ поиска без find и file
ls -Ri ~ 2>/dev/null | awk -v inode="$inode" -v device="$device" '
  /:$/ {
    dir = substr($0, 1, length($0)-1)
    next
  }
  {
    if ($1 == inode) {
      $1 = ""
      sub(/^[[:space:]]+/, "", $0)
      print dir "/" $0
    }
  }
' >> $output_file

echo "Results saved in '$output_file'."
