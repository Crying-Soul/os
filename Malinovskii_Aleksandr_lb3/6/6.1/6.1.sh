#!/bin/bash
# Скрипт демонстрации работы с pipe (каналами) в Linux
# С комментариями и выводом системных ограничений

echo "================================================"
echo " ДЕМОНСТРАЦИЯ РАБОТЫ С PIPE (КАНАЛАМИ) В LINUX"
echo "================================================"
echo

# Раздел 1: Анонимные каналы (pipe)
echo "1. ДЕМОНСТРАЦИЯ АНОНИМНЫХ КАНАЛОВ (PIPE)"
echo "----------------------------------------"

echo "Пример 1.1: Простой pipe между ls и grep"
echo "Команда: ls -l | grep '.txt'"
ls -l | grep ".txt"
echo

echo "Пример 1.2: Цепочка из нескольких команд"
echo "Команда: seq 1 20 | grep '1' | sort -nr"
seq 1 20 | grep "1" | sort -nr
echo

echo "Пример 1.3: Подсчет количества процессов пользователя"
echo "Команда: ps -u $USER | wc -l"
ps -u $USER | wc -l
echo

# Раздел 2: Системные ограничения для pipe
echo "2. СИСТЕМНЫЕ ОГРАНИЧЕНИЯ ДЛЯ PIPE"
echo "----------------------------------"

echo "Текущие ограничения системы для pipe:"
echo

echo "2.1 Максимальный размер буфера pipe:"
echo "Команда: cat /proc/sys/fs/pipe-max-size"
PIPE_MAX_SIZE=$(cat /proc/sys/fs/pipe-max-size 2>/dev/null)
echo "PIPE_MAX_SIZE: ${PIPE_MAX_SIZE:-Недоступно}"
echo "Это значение можно изменить через /etc/sysctl.conf"
echo

echo "2.2 Ограничения на количество pipe для пользователя:"
echo "Команда: cat /proc/sys/fs/pipe-user-pages-{hard,soft}"
PIPE_USER_HARD=$(cat /proc/sys/fs/pipe-user-pages-hard 2>/dev/null)
PIPE_USER_SOFT=$(cat /proc/sys/fs/pipe-user-pages-soft 2>/dev/null)
echo "PIPE_USER_HARD: ${PIPE_USER_HARD:-Недоступно}"
echo "PIPE_USER_SOFT: ${PIPE_USER_SOFT:-Недоступно}"
echo

echo "2.3 Общие ограничения на открытые файлы (включая pipe):"
echo "Команда: ulimit -n"
ULIMIT_N=$(ulimit -n)
echo "ULIMIT -n: $ULIMIT_N"
echo "Это значение можно настроить в /etc/security/limits.conf"
echo

echo "2.4 Размер страницы памяти:"
echo "Команда: getconf PAGESIZE"
PAGESIZE=$(getconf PAGESIZE)
echo "PAGESIZE: $PAGESIZE байт"
echo

# Раздел 3: Просмотр конфигурационных файлов с ограничениями
echo "3. КОНФИГУРАЦИОННЫЕ ФАЙЛЫ ДЛЯ НАСТРОЙКИ PIPE"
echo "--------------------------------------------"

echo "3.3 Дополнительные файлы с ограничениями:"
echo "Файл: /proc/sys/fs/pipe-max-size"
echo "Значение: $(cat /proc/sys/fs/pipe-max-size 2>/dev/null || echo Недоступно)"
echo

echo "3.4 Информация о текущем использовании pipe:"
echo "Команда: lsof | grep 'pipe' | wc -l"
echo "Текущее количество открытых pipe: $(lsof | grep 'pipe' | wc -l)"
echo

