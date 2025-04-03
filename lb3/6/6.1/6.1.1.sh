#!/bin/bash
# Скрипт демонстрации работы с pipe (каналами) в Linux
# Включает примеры использования, системные ограничения и конфигурацию

# Функция для вывода заголовка раздела
print_header() {
    echo
    echo "================================================"
    echo " $1"
    echo "================================================"
    echo
}

# Функция для безопасного чтения системных файлов
safe_read() {
    local file=$1
    if [ -f "$file" ] && [ -r "$file" ]; then
        cat "$file"
    else
        echo "Недоступно (файл не существует или нет прав доступа)"
    fi
}

clear
print_header "ДЕМОНСТРАЦИЯ РАБОТЫ С PIPE (КАНАЛАМИ) В LINUX"

# Раздел 1: Анонимные каналы (pipe)
print_header "1. ДЕМОНСТРАЦИЯ АНОНИМНЫХ КАНАЛОВ (PIPE)"

echo "Пример 1.1: Простой pipe между командами"
echo "Команда: ls -l | grep '.txt'"
ls -l | grep ".txt" 2>/dev/null || echo "Файлы .txt не найдены"
echo

echo "Пример 1.2: Цепочка из нескольких команд с обработкой данных"
echo "Команда: seq 1 20 | grep '1' | sort -nr | tee filtered_numbers.txt"
seq 1 20 | grep "1" | sort -nr | tee filtered_numbers.txt
echo "Результат сохранен в filtered_numbers.txt"
echo

echo "Пример 1.3: Подсчет количества процессов пользователя"
echo "Команда: ps -u $USER -o pid= | wc -l"
process_count=$(ps -u "$USER" -o pid= | wc -l)
echo "Количество процессов: $process_count"
echo

echo "Пример 1.4: Использование pipe с awk для сложной обработки"
echo "Команда: df -h | awk '\$6 == \"/\" {print \"Свободно на корневом разделе: \" \$4}'"
df -h | awk '$6 == "/" {print "Свободно на корневом разделе: " $4}'
echo

# Раздел 2: Системные ограничения для pipe
print_header "2. СИСТЕМНЫЕ ОГРАНИЧЕНИЯ ДЛЯ PIPE"

echo "2.1 Максимальный размер буфера pipe (в байтах):"
pipe_max_size=$(safe_read "/proc/sys/fs/pipe-max-size")
echo "PIPE_MAX_SIZE: $pipe_max_size"
echo "Для изменения добавьте в /etc/sysctl.conf:"
echo "fs.pipe-max-size = <новое_значение>"
echo

echo "2.2 Ограничения на количество pipe для пользователя:"
pipe_user_hard=$(safe_read "/proc/sys/fs/pipe-user-pages-hard")
pipe_user_soft=$(safe_read "/proc/sys/fs/pipe-user-pages-soft")
echo "PIPE_USER_HARD (максимальное количество страниц): $pipe_user_hard"
echo "PIPE_USER_SOFT (мягкое ограничение страниц): $pipe_user_soft"
echo

echo "2.3 Общие ограничения на открытые файлы (включая pipe):"
ulimit_n=$(ulimit -n)
echo "ULIMIT -n (максимум открытых файлов): $ulimit_n"
echo "Для изменения создайте /etc/security/limits.d/custom.conf с содержимым:"
echo "$USER soft nofile 16384"
echo "$USER hard nofile 32768"
echo

echo "2.4 Размер страницы памяти:"
pagesize=$(getconf PAGESIZE)
echo "PAGESIZE: $pagesize байт"
echo "Это влияет на эффективность работы с pipe"
echo

# Раздел 3: Просмотр конфигурационных файлов с ограничениями
print_header "3. КОНФИГУРАЦИОННЫЕ ФАЙЛЫ ДЛЯ НАСТРОЙКИ PIPE"

echo "3.1 Основные файлы с ограничениями:"
echo "/proc/sys/fs/pipe-max-size: $(safe_read "/proc/sys/fs/pipe-max-size")"
echo "/proc/sys/fs/pipe-user-pages-soft: $(safe_read "/proc/sys/fs/pipe-user-pages-soft")"
echo "/proc/sys/fs/pipe-user-pages-hard: $(safe_read "/proc/sys/fs/pipe-user-pages-hard")"
echo

echo "3.2 Текущее использование pipe в системе:"
echo "Команда: lsof | grep -w 'pipe' | wc -l"
pipe_count=$(lsof 2>/dev/null | grep -w 'pipe' | wc -l)
echo "Текущее количество открытых pipe: $pipe_count"
echo

echo "3.3 Пример настройки ограничений через sysctl:"
echo "Для увеличения размера pipe добавьте в /etc/sysctl.conf:"
echo "fs.pipe-max-size = 1048576"
echo "И выполните: sysctl -p"
echo

# Раздел 4: Дополнительные примеры использования
print_header "4. ДОПОЛНИТЕЛЬНЫЕ ПРИМЕРЫ ИСПОЛЬЗОВАНИЯ PIPE"

echo "Пример 4.1: Параллельная обработка с pipe"
echo "Команда: seq 1 10 | xargs -P 4 -I {} echo \"Обработка {}\""
seq 1 10 | xargs -P 4 -I {} echo "Обработка {}"
echo

