#!/bin/bash

# Функция для проверки, является ли процесс системным
is_system_process() {
    local username=$1
    local exe_path=$2

    # Системные процессы обычно запущены от имени системных пользователей
    if [[ "$username" == "root" || "$username" == "SYSTEM" || "$username" == "LOCAL SERVICE" || "$username" == "NETWORK SERVICE" ]]; then
        return 0
    fi

    # Системные процессы часто находятся в системных директориях
    if [[ "$exe_path" == /usr/bin/* || "$exe_path" == /usr/sbin/* || "$exe_path" == /bin/* || "$exe_path" == /sbin/* || "$exe_path" == /lib/* || "$exe_path" == /usr/lib/* ]]; then
        return 0
    fi

    return 1
}

# Получаем список всех процессов
echo "Анализ процессов..."
echo "===================="

# Используем команду ps для получения информации о процессах
ps -eo pid,user,comm,args | while read -r pid user comm args; do
    # Пропускаем заголовок
    if [[ "$pid" == "PID" ]]; then
        continue
    fi

    # Получаем полный путь к исполняемому файлу
    exe_path=$(readlink -f /proc/$pid/exe 2>/dev/null)

    # Проверяем, является ли процесс системным
    if is_system_process "$user" "$exe_path"; then
        echo "Системный процесс: PID=$pid, Пользователь=$user, Команда=$comm, Исполняемый файл=$exe_path"
    else
        echo "Пользовательский процесс: PID=$pid, Пользователь=$user, Команда=$comm, Исполняемый файл=$exe_path"
    fi
done