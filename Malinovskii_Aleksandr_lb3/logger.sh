#!/bin/bash
set -euo pipefail

# Описание: Логгер для отчётности по лабораторной работе.
# Если скрипт не запущен внутри сессии script, то запускаем его через script.
if [ -z "${IN_SCRIPT-}" ]; then
    export IN_SCRIPT=1
    export LOG_FILE="lab_report.log"
    > "$LOG_FILE"  # Очищаем лог-файл перед новым запуском
    script -a -q -c "$0" "$LOG_FILE"
    exit
fi

execute_commands() {

    local task_number=$1
    shift
    local commands=("$@")

    local start_time end_time duration
    start_time=$(date +%s)

     local current_path=$(pwd)
    local short_path="${current_path/#$HOME/~}"
    for cmd in "${commands[@]}"; do
        run_and_log "$(pwd)/$ $cmd " bash -c "$cmd"
    done
    end_time=$(date +%s)
    duration=$((end_time - start_time))
    #log "Время выполнения задания ${task_number}: ${duration} секунд."
}


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
    while IFS= read -r line; do
        echo "$line"
    done < <("$@" 2>&1)
}

# Основные переменные
OUTPUT_FILE="out.txt"
LB_FOLDER="."

# Заголовок отчёта
cat <<EOF
Дата исполнения отчёта: $(date '+%Y-%m-%d %H:%M:%S')
Идентификация: $(whoami)
Малиновский Александр Алексеевич
EOF

# Профиль системы
log "ПРОФИЛЬ СИСТЕМЫ"
run_and_log "lscpu" lscpu
run_and_log "free -h" free -h
run_and_log "lsblk" lsblk
run_and_log "ip a" ip a

# Идентификация виртуальной машины
if systemd-detect-virt --quiet; then
    log "Тип виртуализации: $(systemd-detect-virt)"
else
    log "Система запущена на физическом оборудовании."
fi

# Идентификация/описание ОС
log "ИДЕНТИФИКАЦИЯ/ОПИСАНИЕ ОС"
run_and_log "uname -a" uname -a
if command -v lsb_release >/dev/null 2>&1; then
    run_and_log "lsb_release -a" lsb_release -a
else
    log "Команда lsb_release не найдена."
fi

# Функция для выполнения задания с замером времени выполнения
find_script() {
    local script_name=$1
    local search_dir=${2:-.}  # По умолчанию поиск начинается с текущей директории

    # Поиск файла с заданным именем в директории и всех поддиректориях
    local found_script=$(find "$search_dir" -type f -name "$script_name" -print -quit)

    if [[ -n "$found_script" ]]; then
        echo "$found_script"
        return 0
    else
        echo "Скрипт '$script_name' не найден."
        return 1
    fi
}

execute_task() {
    local task_number=$1
    local output_file_flag=${2:-1}  # По умолчанию 1 (выводить out.txt)
    shift 2
    local task_script_name="${task_number}.sh"
    local task_script=$(find_script "$task_script_name" "$LB_FOLDER")
    local task_args=("$@")

    if [[ -z "$task_script" ]]; then
        log "Скрипт для задания ${task_number} не найден. Пропуск задания."
        return 1
    fi

    #log "----------------- ЗАДАНИЕ ${task_number} -------------------------"
    local current_path=$(pwd)
    local short_path="${current_path/#$HOME/~}"
    
    # Формируем строку пользователь@хост:путь
    local user_host_path="immortal@$(hostname):${short_path}"
    
    if [[ ${#task_args[@]} -gt 0 ]]; then
        log "${user_host_path}\$ ${task_script} ${task_args[*]}"
    else
        log "${user_host_path}\$ ${task_script}"
    fi
    # Состояние файла до выполнения
    if [[ "$output_file_flag" -eq 1 && -f "$OUTPUT_FILE" ]]; then
        echo -e "\n>>> СОСТОЯНИЕ ФАЙЛА ДО ВЫПОЛНЕНИЯ $OUTPUT_FILE <<<"
        echo "----------------------------------------"
        if [[ ! -s "$OUTPUT_FILE" ]]; then
            echo "Файл $OUTPUT_FILE пуст."
        else
            cat "$OUTPUT_FILE"
        fi
        echo "----------------------------------------"
    fi

    local start_time end_time duration
    start_time=$(date +%s)
    bash "$task_script" "${task_args[@]}"
    end_time=$(date +%s)
    duration=$((end_time - start_time))
    #log "Время выполнения задания ${task_number}: ${duration} секунд."

    # Состояние файла после выполнения
    if [[ "$output_file_flag" -eq 1 && -f "$OUTPUT_FILE" ]]; then
        echo -e "\n>>> СОСТОЯНИЕ ФАЙЛА ПОСЛЕ ВЫПОЛНЕНИЯ $OUTPUT_FILE <<<"
        echo "----------------------------------------"
        if [[ ! -s "$OUTPUT_FILE" ]]; then
            echo "Файл $OUTPUT_FILE пуст."
        else
            cat "$OUTPUT_FILE"
        fi
        echo "----------------------------------------"
        >"$OUTPUT_FILE"
    fi
}

task(){
    log "++++++++++++++++++++++++++++++++++++++++++++++++++++++"
    log "----------------- ЗАДАНИЕ $1 -------------------------"
    log "++++++++++++++++++++++++++++++++++++++++++++++++++++++"
}

############### Выполнение заданий
task 1
execute_task 1 0
task 2
#execute_task 2 0
task 3
execute_task 3 0
task 4
execute_task 4 0
task 5
execute_task 5 0
task 6
execute_task 6.1 0
execute_task 6.2 0
execute_task 6.3 0
task 7
execute_task 7 0
execute_task 7.1 0
execute_task 7.2 0
execute_task 7.3 0
execute_task 7.3_bm 0