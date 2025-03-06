#!/bin/bash
set -euo pipefail

# Описание: Логгер для отчётности по лабораторной работе.
# Если скрипт не запущен внутри сессии script, то запускаем его через script.
if [ -z "${IN_SCRIPT-}" ]; then
    export IN_SCRIPT=1
    export LOG_FILE="lab_report.log"
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
    echo 
}

# Основные переменные
OUTPUT_FILE="out.txt"
LB_FOLDER="./"

# Заголовок отчёта
cat <<EOF
=================================================================
                   ОТЧЁТ ПО ЛАБОРАТОРНОЙ РАБОТЕ
=================================================================
Дата исполнения отчёта: $(date '+%Y-%m-%d %H:%M:%S')
Идентификация исполнителя: $(whoami)
Группа: 3343
ФИО: Гребнев Егор Дмитриевич
-----------------------------------------------------------------
EOF

# Профиль системы
log "ПРОФИЛЬ СИСТЕМЫ"
run_and_log ">>> Информация о процессоре (lscpu) <<<" lscpu
run_and_log ">>> Информация о памяти (free -h) <<<" free -h
run_and_log ">>> Информация о дисках (lsblk) <<<" lsblk
run_and_log ">>> Информация о сетевых интерфейсах (ip a) <<<" ip a

# Идентификация виртуальной машины
log "ИДЕНТИФИКАЦИЯ ВИРТУАЛЬНОЙ МАШИНЫ"
if systemd-detect-virt --quiet; then
    log "Система запущена в виртуальной машине."
    log "Тип виртуализации: $(systemd-detect-virt)"
else
    log "Система запущена на физическом оборудовании."
fi
echo -e "\n-----------------------------------------------------------------"

# Идентификация/описание ОС
log "ИДЕНТИФИКАЦИЯ/ОПИСАНИЕ ОС"
run_and_log ">>> Информация о ядре (uname -a) <<<" uname -a
if command -v lsb_release >/dev/null 2>&1; then
    run_and_log ">>> Информация о версии ОС (lsb_release -a) <<<" lsb_release -a
else
    log "Команда lsb_release не найдена."
fi
echo -e "\n-----------------------------------------------------------------"

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

    log "ВЫПОЛНЕНИЕ ЗАДАНИЯ ${task_number} (Скрипт)"
     if [[ ${#task_args[@]} -gt 0 ]]; then
        log "Запуск скрипта ${task_script} с аргументами: ${task_args[*]}"
    else
        log "Запуск скрипта ${task_script} без аргументов."
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
    log "Время выполнения задания ${task_number}: ${duration} секунд."

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



# Функция для выполнения отдельных команд
execute_commands() {

    local task_number=$1
    shift
    local commands=("$@")

    log "ВЫПОЛНЕНИЕ ЗАДАНИЯ ${task_number}"
    # log "Выполнение команд: ${commands[*]}"

    local start_time end_time duration
    start_time=$(date +%s)
    for cmd in "${commands[@]}"; do
        run_and_log ">>> Выполнение команды: $cmd <<<" bash -c "$cmd"
    done
    end_time=$(date +%s)
    duration=$((end_time - start_time))
    log "Время выполнения задания ${task_number}: ${duration} секунд."
}

# # Выполнение заданий
# log "НАЧАЛО ВЫПОЛНЕНИЯ ЗАДАНИЙ"

# execute_task 1.1 0

# # execute_task 1.2 0

# execute_task 1.3 0

# execute_task 1.4 0

# execute_task 1.5 0
 
# execute_commands 2.1 \
#         "./lb2/task-runner.sh ./lb2/2/2.1 2.1.c --run 2.1"

# execute_commands 2.2 \
#         "./lb2/task-runner.sh ./lb2/2/2.2 2.2-father.c 2.2-son.c --run 2.2-father"


execute_task 2.3 0



# Замер общего времени выполнения скрипта
SCRIPT_END=$(date +%s)
TOTAL_DURATION=$((SCRIPT_END - SCRIPT_START))
log "Общее время выполнения скрипта: ${TOTAL_DURATION} секунд."

cat <<EOF
=================================================================
                      ОТЧЁТ ЗАВЕРШЁН
=================================================================
EOF
log "Основной отчёт сохранён в файл: $LOG_FILE"