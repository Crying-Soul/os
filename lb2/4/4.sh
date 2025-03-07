#!/bin/bash

# Файл для логов скрипта
script_file="script_output.log"
: > "$script_file"  # очищаем файл

# Файл для вывода программы 4
program_file="program_out.log"
: > "$program_file"  # очищаем файл

# Функция логирования: вывод в консоль и запись в файл скрипта
log() {
    echo "$1" | tee -a "$script_file"
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

# Запускаем программу 4 в фоне, весь её вывод в program_file
./lb2/4/4 > "$program_file" &
prog_pid=$!
sleep 5

log "----------------------------------------"
log "Анализ нитей и ресурсов процесса ./4"
log "----------------------------------------"

# Получаем PID процесса 4
PID=$(pidof 4)

if [[ -z "$PID" ]]; then
    log "Процесс ./4 не найден"
    exit 1
fi

log "Найден процесс с PID: $PID"
log "----------------------------------------"
log "1. Информация о нитях процесса (ps -Lf):"
log "----------------------------------------"
ps -Lf -p "$PID" | tee -a "$script_file"

log ""
log "----------------------------------------"
log "2. Расширенная информация о нитях (TID, загрузка CPU, распределение по ядрам):"
log "----------------------------------------"
ps -L -o tid,pcpu,psr -p "$PID" | tee -a "$script_file"

log ""
log "----------------------------------------"
log "3. Наследуемые параметры процесса:"
log "----------------------------------------"
ps -p "$PID" -o pid,ppid,uid,gid,comm | tee -a "$script_file"

log ""
log "----------------------------------------"
log "4. Разделяемые ресурсы нитями (открытые файлы):"
log "----------------------------------------"
lsof -p "$PID" 2>/dev/null | tee -a "$script_file"

log ""
log "----------------------------------------"
log "5. Карта памяти процесса (/proc/$PID/maps):"
log "----------------------------------------"
cat /proc/"$PID"/maps | tee -a "$script_file"

log "----------------------------------------"
log "Удаление последнего найденного TID процесса"

# Находим последний TID, связанный с PID
last_tid=$(ps -Lf -p "$PID" | tail -n +2 | awk 'END {print $4}')

if [[ -n "$last_tid" ]]; then
    log "Удаляем последний найденный TID: $last_tid"
    kill -9 "$last_tid" && log "Нить с TID $last_tid успешно удалена. Вся программа завершена."
else
    log "Ошибка: Нить для процесса с PID $PID не найдена."
fi


wait "$prog_pid" 2>/dev/null || true


{
    echo "===== Вывод программы 4 (файл: $program_file) ====="
    cat "$program_file"
    echo ""
    echo "===== Лог скрипта (файл: $script_file) ====="
    cat "$script_file"
} > out.txt

