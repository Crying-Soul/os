#!/bin/bash
set -euo pipefail

# Описание: Логгер для отчётности по лабораторной работе.
# Если скрипт не запущен внутри сессии script, то запускаем его через script.
if [ -z "${IN_SCRIPT-}" ]; then
    export IN_SCRIPT=1
    export LOG_FILE="sys_report.log"
    script -a -q -c "$0" "$LOG_FILE"
    exit
fi

# Замер общего времени выполнения скрипта
SCRIPT_START=$(date +%s)

# Функция для логирования с меткой времени
log() {
    local timestamp
    timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    echo -e "\n[$timestamp] $*"
}

# Функция для выполнения команды и логирования её вывода построчно
run_and_log() {
    local header="$1"
    shift
    log "$header"
    echo -e "\n"
    
    # Формируем строку с путём и пользователем
    local user_path
    user_path="$(whoami)@$(hostname):$(pwd | sed 's|^/home/'"$(whoami)"'|~|')$"
    
    # Выводим строку с путём и выполняем команду
    echo -e "${user_path} $*"
    while IFS= read -r line; do
        echo "$line"
    done < <("$@" 2>&1)
    echo -e "\n"
}

# Основные переменные
OUTPUT_FILE="out.txt"
LB_FOLDER="lb1/"

# Заголовок отчёта
cat <<EOF
Дата исполнения отчёта: $(date '+%Y-%m-%d %H:%M:%S')
Идентификация исполнителя: $(whoami)
Группа: 3343
ФИО: Малиновский Александр Алексеевич
EOF

# Идентификация/описание ОС
log "ИДЕНТИФИКАЦИЯ/ОПИСАНИЕ ОС"
run_and_log ">>> Информация о ядре (uname -a) <<<" uname -a
if command -v lsb_release >/dev/null 2>&1; then
    run_and_log ">>> Информация о версии ОС (lsb_release -a) <<<" lsb_release -a
else
    log "Команда lsb_release не найдена."
fi

# Профиль системы
log "ПРОФИЛЬ СИСТЕМЫ"
run_and_log ">>> lscpu <<<" lscpu
run_and_log ">>> free -h <<<" free -h
run_and_log ">>> (lsblk) <<<" lsblk
run_and_log ">>> (ip a) <<<" ip a

# Идентификация виртуальной машины
if systemd-detect-virt --quiet; then
    log "Система запущена в виртуальной машине."
    log "Тип виртуализации: $(systemd-detect-virt)"
else
    log "Система запущена на физическом оборудовании."
fi

# Функция для выполнения команд и логирования их вывода
execute_commands() {
    local task_number=$1
    shift
    local commands=("$@")

    log "ВЫПОЛНЕНИЕ ЗАДАНИЯ ${task_number}"
    local start_time end_time duration
    start_time=$(date +%s)
    for cmd in "${commands[@]}"; do
        run_and_log ">>> Выполнение команды: $cmd <<<" bash -c "$cmd"
    done
    end_time=$(date +%s)
    duration=$((end_time - start_time))
    log "Время выполнения задания ${task_number}: ${duration} секунд."
}

# Завершение скрипта
SCRIPT_END=$(date +%s)
SCRIPT_DURATION=$((SCRIPT_END - SCRIPT_START))
log "Общее время выполнения скрипта: ${SCRIPT_DURATION} секунд."