#!/bin/bash

# Функция для проверки, является ли процесс системным
is_system_process() {
    local flags=$1

    # Если флаг нечетный, процесс системный
    if (( (flags & 1) == 1 )); then
        return 0
    fi

    return 1
}

# Получаем список всех процессов
echo "Анализ процессов..."
echo "==========================================================="

# Выводим заголовки с выравниванием
printf "%-10s %-15s %-27s %-1s\n" "PID" "Пользователь" "Команда" "Тип"

# Используем команду ps для получения информации о процессах, включая флаги
ps -eo pid,user,comm,flags | while read -r pid user comm flags; do
    # Пропускаем заголовок
    if [[ "$pid" == "PID" ]]; then
        continue
    fi

    # Проверяем, является ли процесс системным
    if is_system_process "$flags"; then
        printf "%-10s %-12s %-20s %-10s\n" "$pid" "$user" "$comm" "Системный"
    else
        printf "%-10s %-12s %-20s %-10s\n" "$pid" "$user" "$comm" "Пользовательский"
    fi
done
