#!/bin/bash

# Выводим только PID, имя процесса и пользователя
echo -e "Системные процессы с привилегиями root:"
ps -eo pid,uid,comm --sort=pid | grep '^ *1\|^ *[0-9]\{1,3\} ' | grep -v 'root' | head -n 10

echo -e "\nПроцессы с PID < 1000 (системные):"
ps -eo pid,comm --sort=pid | awk '$1 < 1000 {print $0}' | head -n 10

echo -e "\nПроцессы, связанные с ядром (например, kworker):"
ps -eo pid,comm --sort=pid | grep 'kworker' | head -n 10

# Процессы, связанные с systemd (иногда являются системными)
echo -e "\nПроцессы, связанные с systemd:"
ps -eo pid,comm --sort=pid | grep 'systemd' | head -n 10