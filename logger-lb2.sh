#!/bin/bash
set -euo pipefail

# Описание: Логгер для отчётности по лабораторной работе.
# Если скрипт не запущен внутри сессии script, то запускаем его через script.
if [ -z "${IN_SCRIPT-}" ]; then
    export IN_SCRIPT=1
    export LOG_FILE="lab_report_$(date +%Y-%m-%d_%H-%M-%S).log"
    script -a -q -c "$0" "$LOG_FILE"
    exit
fi

# Замер общего времени выполнения скрипта
SCRIPT_START=$(date +%s)

timestamp() {
    date '+%Y-%m-%d %H:%M:%S'
}

# Функция для логирования сообщений
log() {
    if [ -n "$1" ]; then
        printf "[%s] %s\n" "$(timestamp)" "$1"
    else
        echo ""
    fi
}

# Функция для логирования комментариев
log_comment() {
    printf "[%s] [КОММЕНТАРИЙ] %s\n" "$(timestamp)" "$*"
}

# Функция для выполнения команды и логирования её вывода
run_and_log() {
    local header="$1"
    shift
    log "$header"

    local output
    output=$("$@" 2>&1)

    # Если вывод не пустой, выводим разделительные полосы и сам вывод
    if [[ -n "$output" ]]; then
        echo "----------------------------------------"
        echo "$output"
        echo "----------------------------------------"
    fi
}

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

display_file_state() {
    local state="$1"
    local file="$2"
    echo -e "\n>>> СОСТОЯНИЕ $file $state СКРИПТА<<<"
    echo "----------------------------------------"
    if [[ ! -s "$file" ]]; then
        echo "Файл $file пуст."
    else
        cat "$file"
    fi
    echo "----------------------------------------"
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


  
    if [[ "$output_file_flag" -eq 1 && -f "$OUTPUT_FILE" ]]; then
        display_file_state "ДО ВЫПОЛНЕНИЯ" "$OUTPUT_FILE"
    fi

     if [[ -f "$task_script" ]]; then
        local start_time end_time duration
        start_time=$(date +%s)
        local output
        output=$(bash "$task_script" "${task_args[@]}" 2>&1)
        if [[ -n "$output" ]]; then
            echo "$output" | sed "s/^/$(basename "$task_script") >> /"
        fi
        end_time=$(date +%s)
        duration=$((end_time - start_time))

        if [[ "$output_file_flag" -eq 1 && -f "$OUTPUT_FILE" ]]; then
            display_file_state "ПОСЛЕ ВЫПОЛНЕНИЯ" "$OUTPUT_FILE"
            >"$OUTPUT_FILE" # Очистка файла после выполнения
        fi
        log "Время выполнения задания ${task_number}: ${duration} секунд."
        log ""
        log ""
    else
        log "Скрипт для задания ${task_number} не найден. Пропуск задания."
        return 1
    fi
}



# Функция для выполнения отдельных команд
execute_commands() {
    local task_number=$1
    shift
    local commands=("$@")

    log "ВЫПОЛНЕНИЕ ЗАДАНИЯ ${task_number}"

    local start_time end_time duration
    start_time=$(date +%s)
    for cmd in "${commands[@]}"; do
        local user_host="$(whoami)@$(hostname)"
        run_and_log ">>> ${user_host}$ $cmd <<<" bash -c "$cmd"
    done
    end_time=$(date +%s)
    duration=$((end_time - start_time))
    log "Время выполнения задания ${task_number}: ${duration} секунд."
    log ""
    log ""
}

# Основные переменные
OUTPUT_FILE="out.txt"
LB_FOLDER="./lb2/"

# Заголовок отчёта
cat <<EOF
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



# Выполнение заданий
log "НАЧАЛО ВЫПОЛНЕНИЯ ЗАДАНИЙ"

log_comment "Выполним подготовку окружения для выполнения заданий. Скомпилируем все исходные файлы."

execute_commands 0 \
        "./compiller.sh lb2"

# execute_task 1.1 0

# # execute_task 1.2 0

# execute_task 1.3 0

# execute_task 1.4 0

# execute_task 1.5 0
 
# execute_commands 2.1 \
        # "./lb2/task-runner.sh ./lb2/2/2.1 2.1.c --run 2.1"

# execute_commands 2.2 \
        # "./lb2/task-runner.sh ./lb2/2/2.2 2.2-father.c 2.2-son.c --run 2.2-father"


# execute_commands 2.3 \
#         "./lb2/task-runner.sh ./lb2/2/2.3 2.3.c --run 2.3"

# execute_commands 2.4 \
#         "./lb2/task-runner.sh ./lb2/2/2.4 2.4-a.c 2.4-b.c 2.4-c.c --run 2.4-a"

# execute_commands 3.1 \
        # "./lb2/task-runner.sh ./lb2/3/3.1 3.1-father.c 3.1-son1.c 3.1-son2.c 3.1-son3.c --run 3.1-father"

# execute_commands 4 \
#         "./lb2/task-runner.sh ./lb2/4 4.c --run 4"

# execute_task 4 0

log_comment "Конец скрипта. Очистка файлов."

execute_commands 0 \
        "./cleaner.sh lb2"


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