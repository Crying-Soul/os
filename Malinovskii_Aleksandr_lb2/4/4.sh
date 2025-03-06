#!/bin/bash
#
set -euo pipefail

# Файл для результатов
result_file="task4-results.txt"
: > "$result_file"  # Очищаем файл

# Функция логирования: вывод в консоль и запись в файл
print() {
    echo "$1" | tee -a "$result_file"
}

# Функция проверки наличия необходимых команд
check_command() {
    if ! command -v "$1" &>/dev/null; then
        print "Ошибка: команда '$1' не найдена. Установите необходимый пакет."
        exit 1
    fi
}

# Проверяем наличие необходимых утилит
for cmd in pgrep ps cat; do
    check_command "$cmd"
done

print "Анализ нитей и ресурсов процесса ./4"

# Получаем PID первого процесса, соответствующего ./task4
PID=$(pidof 4)

if [[ -z "$PID" ]]; then
    print "Процесс ./task4 не найден"
    exit 1
fi

print "Найден процесс с PID: $PID"
print "Информация о нитях процесса (ps -Lf):"
ps -Lf -p "$PID" | tee -a "$result_file"  

print ""
print "Расширенная информация о нитях (TID, загрузка CPU, распределение по ядрам):"
ps -L -o tid,pcpu,psr -p "$PID" | tee -a "$result_file"  

print ""
print "Наследуемые параметры процесса:"
ps -p "$PID" -o pid,ppid,uid,gid,comm | tee -a "$result_file"

print ""
print "Карта памяти процесса (/proc/$PID/maps):"
cat /proc/"$PID"/maps | tee -a "$result_file"

print "Удаление последнего найденного TID процесса"

# Находим последний TID, связанный с PID
last_tid=$(ps -Lf -p "$PID" | tail -n +2 | awk 'END {print $4}')

# Проверяем, что TID найден
if [[ -n "$last_tid" ]]; then
    print "Удаляем последний найденный TID: $last_tid"
    # Попытка удалить нить с помощью kill
    kill -9 "$last_tid" && print "Нить с TID $last_tid успешно удалена."
else
    print "Ошибка: Нить для процесса с PID $PID не найдена."
fi

print "После удаления нити удалился процесс с $PID."