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
    echo "----------------------------------------"
    while IFS= read -r line; do
        echo "$line"
    done < <("$@" 2>&1)
    echo "----------------------------------------"
}

# Основные переменные
OUTPUT_FILE="out.txt"
LB_FOLDER="lb1/"

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
execute_task() {
    local task_number=$1
    local output_file_flag=${2:-1}  # По умолчанию 1 (выводить out.txt)
    shift 2
    local task_script="${LB_FOLDER}task${task_number}.sh"
    local task_args=("$@")

    log "ВЫПОЛНЕНИЕ ЗАДАНИЯ ${task_number} (Скрипт)"
    log "Запуск скрипта ${task_script} с аргументами: ${task_args[*]}"

    if [[ -f "$task_script" ]]; then
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

# Выполнение заданий
log "НАЧАЛО ВЫПОЛНЕНИЯ ЗАДАНИЙ"


# Пример выполнения задания с использованием скрипта
execute_task 1 1 "out.txt"

# Пример выполнения задания с использованием отдельных команд
execute_task 2 1 "./lb1/tests/linked_file.txt"

mkdir -p ./lb1/links1 

execute_commands 3 \
    "ln -s ./lb1/tests/linked_file.txt ./lb1/links1/symlink_ln.txt && ls -l ./lb1/links1/symlink_ln.txt" \
    "cp -s ./lb1/tests/linked_file.txt ./symlink_cp.txt && ls -l ./symlink_cp.txt" \
    "link /lb1/tests/linked_file.txt ./lb1/links1/hardlink.txt && ls -l ./lb1/links1/hardlink.txt"

# Пример выполнения задания с использованием скрипта
execute_task 3 1 "./lb1/tests/linked_file.txt" #symlinks

rm -rf ./lb1/links1 ./symlink_cp.txt ./lb1/tests/hardlink.txt

# Определяем целевой файл
target_file="./lb1/tests/linked_file.txt"

# Выполнение задания с использованием отдельных команд
execute_commands 4 \
    "ls -lRa /home/lotos | grep ' -> ' | grep '$target_file'" \
    "find /home/lotos -type l -exec stat -c '%N' {} \; 2>/dev/null | grep '$target_file'" \
    "find /home/lotos -type l -lname '*$(basename "$target_file")*' 2>/dev/null" \
    "for link in \$(find /home/lotos -type l 2>/dev/null); do if readlink \"\$link\" | grep -q '$target_file'; then echo \"\$link -> \$(readlink \"\$link\")\"; fi; done"

execute_commands 5 \
    "find /home/lotos -type f 2>/dev/null | head -n 3" \
    "find /home/lotos -type d 2>/dev/null | head -n 3" \
    "find /home/lotos -type l 2>/dev/null | head -n 3" \
    "find /home/lotos -type f -mtime -7 -exec stat -c '%n %y' {} + 2>/dev/null | head -n 3" \
    "find /home/lotos -type f -size +1M -exec ls -lh {} + 2>/dev/null | head -n 3"

execute_commands 6 \
    "od -c /home/lotos/test/example.txt | head -n 10" \
    "od -x /home/lotos/test/example.bin | head -n 10" \
    "hexdump -C /home/lotos/test/example_exec | head -n 10" \
    "hexdump -C /home/lotos/test/image.jpg | head -n 10"

execute_task 7 0

execute_commands 8 \
    "ls -l /etc/passwd && stat /etc/passwd" \
    "ls -l /etc/shadow && stat /etc/shadow" \
    "ls -l /usr/bin/passwd && stat /usr/bin/passwd && getcap /usr/bin/passwd" \
    "head -n 5 /etc/passwd" \
    "sudo head -n 5 /etc/shadow" \
    "ls -l /usr/bin/passwd"

rm -rf /tmp/testdir
rm -rf /tmp/testdir2
userdel anotheruser

# 9. Исследование прав владения и доступа
execute_commands 9.1 \
    "useradd -m anotheruser || true" \
    "mkdir -p /tmp/testdir && echo 'Файл 1: содержимое' > /tmp/testdir/file1 && echo 'Файл 2: содержимое' > /tmp/testdir/file2 && echo 'Файл 3: содержимое' > /tmp/testdir/file3 && ls -l /tmp/testdir" \
    "chown root:root /tmp/testdir/file1 && ls -l /tmp/testdir" \
    "chmod 755 /tmp/testdir/file1 && chmod 644 /tmp/testdir/file2 && ls -l /tmp/testdir"

execute_commands 9.2 \
    "touch /tmp/testdir/execfile && echo -e \"#!/bin/bash\necho 'Hello, World!'\" > /tmp/testdir/execfile && chmod +x /tmp/testdir/execfile && ls -l /tmp/testdir" \
    "chmod u+s /tmp/testdir/execfile && ls -l /tmp/testdir/execfile"

execute_commands 9.3 \
    "touch /tmp/testdir/file4 && echo 'Это файл 4' > /tmp/testdir/file4 && chmod 640 /tmp/testdir/file4 && chown root:root /tmp/testdir/file4 && ls -l /tmp/testdir/file4" \
    "su anotheruser -c \"cat /tmp/testdir/file4\""

execute_commands 9.4 \
    "mkdir -p /tmp/testdir2 && echo 'Это файл 5' > /tmp/testdir2/file5" \
    "chmod 700 /tmp/testdir2/file5 && su anotheruser -c \"cat /tmp/testdir2/file5\"" \
    "chmod 777 /tmp/testdir2/file5 && cat /tmp/testdir2/file5"

execute_task 10 0

execute_commands 11.1 \
    "df -h" \
    "lsblk" \
    "cat /etc/fstab" \
    "mount" \
    "blkid"

execute_commands 11.2 \
    "lsblk -f" \
    "mount" \
    "cat /etc/fstab" \
    "diff before.txt after.txt"

execute_task 11.3 0

# 12. Проанализировать и пояснить принцип работы утилиты file.
execute_commands 12 \
    "file -i /bin/bash" \
    "file -b /bin/bash" \
    "file -z /etc/apt/sources.list" \
    "file -v"

execute_commands 12.1 \
    "file /bin/bash" \
    "hexdump -n 16 -C /bin/bash" \
    "ls -l /usr/share/misc/magic && readlink -f /usr/share/misc/magic" \
    

execute_commands 12.2 \
    "file -i /bin/bash" \
    "file -b /bin/bash" \
    "file -s /dev/sda1" \
    "file -k /bin/bash" \
    "file -L /bin/sh"

execute_task 12.3 0
 
# rm example.mytype
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