#!/bin/bash
"""
Функциональность:
1. Анализ процессов Linux по PID с детализацией через /proc
2. Ключевые операции:
   - Получение базовой информации (ps) с форматированным выводом
   - Извлечение расширенных данных из /proc:
     * Полная командная строка (cmdline)
     * Рабочий каталог (cwd)
     * Путь к исполняемому файлу (exe)
     * Переменные окружения (environ)
3. Системные лимиты:
   - Анализ минимального/максимального PID (pid_max)
   - Поиск активного процесса с максимальным PID
   - Проверка системных UID (0-65534)

Архитектурная роль:
- Инструмент для аудита процессов в Linux
- Зависимости:
  * Доступ к /proc (требует root для некоторых данных)
  * GNU coreutils (ps, awk)
  * getent для работы с UID

"""
# Функция для получения подробной информации о процессе по PID
# Функция для получения подробной информации о процессе по PID
get_process_info() {
    local pid=$1

    # Проверка существования процесса
    if ps -p $pid > /dev/null 2>&1; then
        echo "Подробная информация о процессе с PID $pid:"
        echo "----------------------------------------"
        
        # Основная информация о процессе с заголовками
        echo "  PID   UID  COMMAND         STAT  START     RSS   PPID   NI"
        ps -p $pid -o pid=,uid=,comm=,stat=,start_time=,rss=,ppid=,ni= --no-headers | \
            awk '{printf "  %-5s %-4s %-15s %-5s %-10s %-6s %-6s %-3s\n", $1, $2, $3, $4, $5, $6, $7, $8}'
        
        # Дополнительная информация из /proc
        if [ -d /proc/$pid ]; then
            # Команда запуска
            cmdline=$(cat /proc/$pid/cmdline | tr '\0' ' ')
            echo "Команда запуска: $cmdline"

            # Текущий рабочий каталог
            cwd=$(readlink /proc/$pid/cwd)
            echo "Текущий рабочий каталог: $cwd"

            # Исполняемый файл
            exe=$(readlink /proc/$pid/exe)
            echo "Исполняемый файл: $exe"

            # Окружение процесса
            if [ -f /proc/$pid/environ ]; then
                echo "Окружение процесса:"
                cat /proc/$pid/environ | tr '\0' '\n'
            else
                echo "Окружение процесса недоступно."
            fi
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
echo -e "Максимальный возможный PID: $max_pid\n"


largest_pid=$(ps -e --sort=-pid | head -n 7 | tail -n 1 | awk '{print $1}')
echo "Самый большой активный PID в системе: $largest_pid"
get_process_info $largest_pid


min_uid=0
min_uid_user=$(getent passwd $min_uid | cut -d: -f1)
echo "Минимальный UID: $min_uid"
echo "Пользователь с минимальным UID: $min_uid_user"

# Максимальный UID (обычно это 65534, но зависит от системы)
max_uid=65534
max_uid_user=$(getent passwd $max_uid | cut -d: -f1)
echo "Максимальный UID: $max_uid"
echo "Пользователь с максимальным UID: $max_uid_user"