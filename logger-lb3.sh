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
    output=$("$@")

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
    local search_dir=${2:-.} # По умолчанию поиск начинается с текущей директории

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

    shift 1
    local task_script_name="${task_number}.sh"
    local task_script=$(find_script "$task_script_name" "$LB_FOLDER" 2>&1)
    local task_args=("$@")

    if [[ -z "$task_script" ]]; then
        log "Скрипт для задания ${task_number} не найден. Пропуск задания." >&2
        return 1
    fi

    log "ВЫПОЛНЕНИЕ ЗАДАНИЯ ${task_number} (Скрипт)" 2>&1
    if [[ ${#task_args[@]} -gt 0 ]]; then
        log "Запуск скрипта ${task_script} с аргументами: ${task_args[*]}" 2>&1
    else
        log "Запуск скрипта ${task_script} без аргументов." 2>&1
    fi



    if [[ -f "$task_script" ]]; then
        local start_time end_time duration
        start_time=$(date +%s)

        # Перенаправляем весь вывод (stdout+stderr) через tee для отображения и логирования
        bash "$task_script" "${task_args[@]}" 2>&1 
        local exit_code=${PIPESTATUS[0]}

        end_time=$(date +%s)
        duration=$((end_time - start_time))

  
        log "Время выполнения задания ${task_number}: ${duration} секунд." 2>&1
        log "" 2>&1
        log "" 2>&1
        
        return $exit_code
    else
        log "Скрипт для задания ${task_number} не найден. Пропуск задания." >&2
        return 1
    fi
}

execute_commands() {
    local task_number=$1
    shift
    local commands=("$@")

    log "ВЫПОЛНЕНИЕ ЗАДАНИЯ ${task_number}" 2>&1

    local start_time end_time duration
    start_time=$(date +%s)
    
    for cmd in "${commands[@]}"; do
        local user_host="$(whoami)@$(hostname)"
       
        log ">>> ${user_host}$ $cmd <<<" 2>&1
        bash -c "$cmd" 2>&1 
        local exit_code=${PIPESTATUS[0]}
        
        if [ $exit_code -ne 0 ]; then
            log "Ошибка выполнения команды: $cmd (код: $exit_code)" >&2
        fi
    done
    
    end_time=$(date +%s)
    duration=$((end_time - start_time))
    log "Время выполнения задания ${task_number}: ${duration} секунд." 2>&1
    log "" 2>&1
    log "" 2>&1
    
    return $exit_code
}

# Основные переменные
OUTPUT_FILE="out.txt"
LB_FOLDER="./lb3/"

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
    "./compiller.sh lb3"

execute_commands 1.1 \
    "strace -tt -e 'trace=signal,kill,tgkill,tkill,sigaction,sigprocmask' -f ./lb3/1"

execute_commands 1.2 \
    "strace -tt -e 'trace=signal,kill,tgkill,tkill,sigaction,sigprocmask' -f ./lb3/1 --custom-handler"

# execute_task 2
# lotos@ASPIRIN:~/Documents/os$ ./lb3/2 --process_single
# Режим process_single. Нажмите Ctrl+C.
# ^CПроцесс: получен SIGINT (однократно).
# ^C
# lotos@ASPIRIN:~/Documents/os$ ./lb3/2 --process_multi
# Режим process_multi. Нажмите Ctrl+C несколько раз.
# ^CПроцесс: получен SIGINT 1 раз(а).
# ^CПроцесс: получен SIGINT 2 раз(а).
# ^CПроцесс: получен SIGINT 3 раз(а).
# Восстанавливаем исходный обработчик после 3 срабатываний.
# ^C
# lotos@ASPIRIN:~/Documents/os$ ./lb3/2 --thread_single
# Режим thread_single. Нажмите Ctrl+C.
# ^CПоток: получен SIGINT (однократно).
# lotos@ASPIRIN:~/Documents/os$ ./lb3/2 --thread_multi
# Режим thread_multi. Нажмите Ctrl+C несколько раз.
# ^CПоток: получен SIGINT 1 раз(а).
# ^CПоток: получен SIGINT 2 раз(а).
# ^CПоток: получен SIGINT 3 раз(а).
# Поток: после 3 срабатываний восстанавливаем исходное поведение.
# lotos@ASPIRIN:~/Documents/os$ ./lb3/2 --custom_signal
# Режим custom_signal. Нажмите Ctrl+Z.
# ^ZПолучен SIGTSTP (Ctrl+Z)!
# ^C
# lotos@ASPIRIN:~/Documents/os$ 


execute_task 3

execute_commands 4 \
    "strace -tt -v -s 1000 -e trace=signal ./lb3/4"

log_comment "POSIX Вариант"
execute_commands 5.1 \
    "strace -f -e trace=read,write,futex -s 1000 ./lb3/5/5-posix 5"

log_comment "SYSTEMV Вариант"
execute_commands 5.2 \
    "strace -f -e trace=read,write,shmget,shmat,shmdt,semop,semctl,futex ./lb3/5/5-systemv 5"

log_comment "PTHREAD Вариант"
execute_commands 5.2 \
    "strace -f -e trace=read,write,futex ./lb3/5/5-pthread 5"

log_comment "Сравнение на многоядерном процессоре"
execute_commands 5 \
    "./lb3/5/5.sh"

log_comment "Сравнение на одноядерном процессоре"
execute_commands 5 \
    "taskset -c 0 ./lb3/5/5.sh"

execute_task 6.1.1

execute_task 6.1.2

execute_task 6.2

execute_task 6.3

execute_task 7

execute_task 7.1

execute_task 7.2

execute_task 7.3


log_comment "Конец скрипта. Очистка файлов."

execute_commands 0 \
    "./cleaner.sh lb3"\

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
