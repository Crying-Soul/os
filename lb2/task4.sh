#!/bin/bash
set -euo pipefail

# Файл для результатов
result_file="task4-results.txt"
: > "$result_file"  # очищаем файл

# Функция логирования: вывод в консоль и запись в файл
log() {
    echo "$1" | tee -a "$result_file"
}

# Функция проверки наличия необходимых команд
check_command() {
    if ! command -v "$1" &>/dev/null; then
        log "Ошибка: команда '$1' не найдена. Установите необходимый пакет."
        exit 1
    fi
}

# Проверяем наличие необходимых утилит
for cmd in pgrep ps lsof cat; do
    check_command "$cmd"
done

# Опционально запускаем программу task4 (раскомментируйте строки, если требуется)
# ./task4 &
# sleep 5

log "----------------------------------------"
log "Анализ нитей и ресурсов процесса ./task4"
log "----------------------------------------"

# Получаем PID первого процесса, соответствующего ./task4
PID=$(pidof task4)

if [[ -z "$PID" ]]; then
    log "Процесс ./task4 не найден"
    exit 1
fi

log "Найден процесс с PID: $PID"
log "----------------------------------------"
log "1. Информация о нитях процесса (ps -Lf):"
log "----------------------------------------"
ps -Lf -p "$PID" | tee -a "$result_file"  

log ""
log "----------------------------------------"
log "2. Расширенная информация о нитях (TID, загрузка CPU, распределение по ядрам):"
log "----------------------------------------"
ps -L -o tid,pcpu,psr -p "$PID" | tee -a "$result_file"  

log ""
log "----------------------------------------"
log "3. Наследуемые параметры процесса:"
log "----------------------------------------"
ps -p "$PID" -o pid,ppid,uid,gid,comm | tee -a "$result_file"

log ""
log "----------------------------------------"
log "4. Разделяемые ресурсы нитями (открытые файлы):"
log "----------------------------------------"
lsof -p "$PID" 2>/dev/null | tee -a "$result_file"

log ""
log "----------------------------------------"
log "5. Карта памяти процесса (/proc/$PID/maps):"
log "----------------------------------------"
cat /proc/"$PID"/maps | tee -a "$result_file"

log "----------------------------------------"
log "Удаление последнего найденного TID процесса"

# Находим последний TID, связанный с PID
last_tid=$(ps -Lf -p "$PID" | tail -n +2 | awk 'END {print $4}')

# Проверяем, что TID найден
if [[ -n "$last_tid" ]]; then
    log "Удаляем последний найденный TID: $last_tid"
    # Попытка удалить нить с помощью kill
    kill -9 "$last_tid" && log "Нить с TID $last_tid успешно удалена."
else
    log "Ошибка: Нить для процесса с PID $PID не найдена."
fi

# sleep 5
# log "----------------------------------------"
# log "7. Информация о нитях процесса (ps -Lf):"
# log "----------------------------------------"
# ps -Lf -p "$PID" | tee -a "$result_file"  # Пропускаем заголовок

# sleep 5

# Удаляем сам процесс

kill -SIGTERM $(pidof task4)

