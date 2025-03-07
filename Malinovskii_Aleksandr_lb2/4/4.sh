#!/bin/bash
#
set -euo pipefail

# Файл для записи результатов
output_file="out.txt"
: > "$output_file"  # Очищаем файл перед началом записи

# Функция для вывода сообщений и записи в файл
log_message() {
    echo "$1" | tee -a "$output_file"
}

# Функция для проверки доступности команд
verify_command() {
    if ! command -v "$1" &>/dev/null; then
        log_message "Ошибка: команда '$1' недоступна. Установите необходимый пакет."
        exit 1
    fi
}

# Проверяем наличие необходимых утилит
for tool in pgrep ps cat; do
    verify_command "$tool"
done

log_message "Анализ потоков и ресурсов процесса ./4"

# Получаем PID процесса, соответствующего ./4
PROCESS_PID=$(pidof 4)

if [[ -z "$PROCESS_PID" ]]; then
    log_message "Процесс ./4 не обнаружен."
    exit 1
fi

log_message "Обнаружен процесс с PID: $PROCESS_PID"
log_message "Информация о потоках процесса (ps -Lf):"
ps -Lf -p "$PROCESS_PID" | tee -a "$output_file"  

log_message ""
log_message "Детальная информация о потоках (TID, загрузка CPU, распределение по ядрам):"
ps -L -o tid,pcpu,psr -p "$PROCESS_PID" | tee -a "$output_file"  

log_message ""
log_message "Наследуемые параметры процесса:"
ps -p "$PROCESS_PID" -o pid,ppid,uid,gid,comm | tee -a "$output_file"

log_message ""
log_message "Карта памяти процесса (/proc/$PROCESS_PID/maps):"
cat /proc/"$PROCESS_PID"/maps | tee -a "$output_file"

log_message "Попытка завершения последнего обнаруженного потока"

# Находим последний TID, связанный с PROCESS_PID
LAST_TID=$(ps -Lf -p "$PROCESS_PID" | tail -n +2 | awk 'END {print $4}')

# Проверяем, что TID найден
if [[ -n "$LAST_TID" ]]; then
    log_message "Завершаем поток с TID: $LAST_TID"
    # Попытка завершить поток с помощью kill
    if kill -9 "$LAST_TID"; then
        log_message "Поток с TID $LAST_TID успешно завершен."
    else
        log_message "Ошибка: Не удалось завершить поток с TID $LAST_TID."
    fi
else
    log_message "Ошибка: Поток для процесса с PID $PROCESS_PID не обнаружен."
fi

log_message "После завершения потока процесс с PID $PROCESS_PID был остановлен."

сat out.txt
rm out.txt
rm ./4