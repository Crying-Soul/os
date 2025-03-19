#!/bin/bash

# Файл для логов скрипта
SCRIPT_LOG="script_output.log"
> "$SCRIPT_LOG"  # Очищаем файл логов

# Файл для вывода программы 4
PROGRAM_OUTPUT="program_out.log"
> "$PROGRAM_OUTPUT"  # Очищаем файл вывода программы

# Функция для логирования: вывод в консоль и запись в файл
log_message() {
    echo "$1" | tee -a "$SCRIPT_LOG"
}

# Функция для проверки наличия необходимых команд
check_command_availability() {
    if ! command -v "$1" &>/dev/null; then
        log_message "Ошибка: команда '$1' не найдена. Установите необходимый пакет."
        exit 1
    fi
}

# Запускаем программу 4 в фоне, весь её вывод в PROGRAM_OUTPUT
./4/4 > "$PROGRAM_OUTPUT" &
PROGRAM_PID=$!
sleep 5  # Даем программе время на запуск

log_message "========================================"
log_message "Анализ нитей и ресурсов процесса ./4"
log_message "========================================"

# Получаем PID процесса 4
PROGRAM_PID=$(pidof 4)

if [[ -z "$PROGRAM_PID" ]]; then
    log_message "Процесс ./4 не найден."
    exit 1
fi

log_message "=================== Задание 4.2 ======================="
log_message "Найден процесс с PID: $PROGRAM_PID"
log_message " Информация о нитях процесса (ps -Lf):"
ps -Lf -p "$PROGRAM_PID" | tee -a "$SCRIPT_LOG"

log_message ""
log_message " Расширенная информация о нитях (TID, загрузка CPU, распределение по ядрам):"
ps -L -o tid,pcpu,psr -p "$PROGRAM_PID" | tee -a "$SCRIPT_LOG"

log_message ""
log_message " Наследуемые параметры процесса:"
ps -p "$PROGRAM_PID" -o pid,ppid,uid,gid,comm | tee -a "$SCRIPT_LOG"

log_message "=================== Задание 4.3 ======================="
log_message ""
log_message " Разделяемые ресурсы нитями (открытые файлы):"
lsof -p "$PROGRAM_PID" 2>/dev/null | tee -a "$SCRIPT_LOG"

log_message ""
log_message "Карта памяти процесса (/proc/$PROGRAM_PID/maps):"
cat /proc/"$PROGRAM_PID"/maps | tee -a "$SCRIPT_LOG"

log_message "Удаление последнего найденного TID процесса"

log_message "=================== Задание 4.4 ======================="
# Находим последний TID, связанный с PID
LAST_TID=$(ps -Lf -p "$PROGRAM_PID" | tail -n +2 | awk 'END {print $4}')

if [[ -n "$LAST_TID" ]]; then
    log_message "Удаляем последний найденный TID: $LAST_TID"
    if kill -9 "$LAST_TID"; then
        log_message "Нить с TID $LAST_TID успешно удалена. Вся программа завершена."
    else
        log_message "Ошибка: не удалось удалить нить с TID $LAST_TID."
    fi
else
    log_message "Ошибка: Нить для процесса с PID $PROGRAM_PID не найдена."
fi

wait "$PROGRAM_PID" 2>/dev/null || true

# Собираем итоговый отчет
{
    echo "===== Вывод программы 4 (файл: $PROGRAM_OUTPUT) ====="
    cat "$PROGRAM_OUTPUT"
    echo ""
    echo "===== Лог скрипта (файл: $SCRIPT_LOG) ====="
    cat "$SCRIPT_LOG"
} > final_report.txt

log_message "Анализ завершен. Итоговый отчет сохранен в final_report.txt."