#!/bin/bash

# Функция для получения подробной информации о процессе по PID
get_process_info() {
    local pid=$1
    if ps -p $pid > /dev/null 2>&1; then
        echo "Подробная информация о процессе с PID $pid:"
        echo "----------------------------------------"
        # Основная информация о процессе с заголовками
        echo "  PID   UID  COMMAND         STAT START   RSS  PPID  NI"
        ps -p $pid -o pid=,uid=,comm=,stat=,start_time=,rss=,ppid=,ni= --no-headers | awk '{printf "  %-5s %-4s %-15s %-4s %-7s %-5s %-5s %-3s\n", $1, $2, $3, $4, $5, $6, $7, $8}'
        # Дополнительная информация из /proc
        if [ -d /proc/$pid ]; then
            echo "Команда запуска: $(cat /proc/$pid/cmdline | tr '\0' ' ')"
            echo "Текущий рабочий каталог: $(readlink /proc/$pid/cwd)"
            echo "Исполняемый файл: $(readlink /proc/$pid/exe)"
            echo "Окружение процесса:"
            cat /proc/$pid/environ | tr '\0' '\n'
        else
            echo "Информация из /proc/$pid недоступна."
        fi
    else
        echo "Процесс с PID $pid не существует."
    fi
    echo
}

# Минимальный PID
min_pid=1
echo "Минимальный PID: $min_pid"
get_process_info $min_pid

# Максимальный PID
max_pid=$(cat /proc/sys/kernel/pid_max)
echo "Максимальный PID: $max_pid"
get_process_info $max_pid

# Минимальный UID
min_uid=0
min_uid_user=$(getent passwd $min_uid | cut -d: -f1)
echo "Минимальный UID: $min_uid"
echo "Пользователь с минимальным UID: $min_uid_user"