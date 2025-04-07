#!/bin/bash
# =============================================================================
# Скрипт для анализа системных ограничений, связанных с сокетами
# =============================================================================
# Цель:
#   Сбор и отображение информации о системных ограничениях, которые могут
#   влиять на работу сетевых приложений, использующих сокеты (TCP и UDP).
#   Скрипт предоставляет обзор лимитов файловых дескрипторов, диапазонов
#   портов, размеров буферов, очередей соединений, а также информацию о
#   настройках безопасности (firewall, SELinux/AppArmor) и cgroup.
#
# Описание:
#   Скрипт выполняет следующие действия:
#     1. Отображает лимиты файловых дескрипторов (глобальный максимум,
#        максимум на процесс, текущий лимит).
#     2. Отображает диапазоны портов (динамические порты, время TIME_WAIT).
#     3. Отображает размеры буферов (максимальный буфер чтения, максимальный
#        буфер записи, TCP rmem, TCP wmem).
#     4. Отображает параметры очередей соединений (максимальная очередь
#        подключений, SYN backlog).
#     5. Проверяет состояние firewall (iptables и nftables) и отображает
#        первые несколько правил.
#     6. Проверяет состояние системы мандатного контроля доступа (SELinux или
#        AppArmor).
#     7. Отображает ограничения cgroup (если они настроены).
#     8. Отображает общее количество открытых сокетов (TCP, UDP, UNIX).
#     9. Отображает информацию о текущем процессе (PID и лимиты).
#    10. Выводит подробный список сокетов.
#
# Используемые инструменты:
#   - `cat`:  Утилита для вывода содержимого файлов.
#   - `ulimit`: Утилита для управления лимитами ресурсов.
#   - `iptables`: Утилита для управления правилами firewall (может отсутствовать).
#   - `nft`:  Утилита для управления правилами firewall (может отсутствовать).
#   - `sestatus`: Утилита для проверки статуса SELinux (может отсутствовать).
#   - `aa-status`: Утилита для проверки статуса AppArmor (может отсутствовать).
#   - `ss`:   Утилита для отображения информации о сокетах.
#   - `grep`:  Утилита для поиска текста.
#   - `wc`:   Утилита для подсчета слов, строк, символов.
#   - `head`:  Утилита для вывода первых строк файла.
#
# Параметры:
#   Нет. Скрипт не принимает аргументов командной строки.
#
# Зависимости:
#   - coreutils (cat, grep, wc, head, if, test)
#   - net-tools (ss)
#   - iptables (может отсутствовать)
#   - nftables (может отсутствовать)
#   - selinux-utils (может отсутствовать)
#   - apparmor-utils (может отсутствовать)
#
# Вывод:
#   Скрипт выводит информацию о системных ограничениях и состоянии сетевых
#   служб.
#
# Примечания:
#   - Некоторые утилиты (iptables, nft, sestatus, aa-status) могут быть не
#     установлены в системе. В этом случае скрипт выведет соответствующее
#     сообщение.
#   - Значения, полученные из файлов в /proc, могут меняться в зависимости от
#     настроек системы и активности других процессов.
# =============================================================================

echo -e "\n=== Ограничения системы для сокетов ===\n"

# 1. Общие лимиты файловых дескрипторов
echo -e "1. Лимиты файловых дескрипторов:"
echo "• Глобальный максимум (/proc/sys/fs/file-max): $(cat /proc/sys/fs/file-max)"
echo "• Макс. на процесс (/proc/sys/fs/nr_open): $(cat /proc/sys/fs/nr_open)"
echo "• Текущий лимит (ulimit -n): $(ulimit -n)"

# 2. Ограничения портов
echo -e "\n2. Диапазоны портов:"
echo "• Динамические порты (/proc/sys/net/ipv4/ip_local_port_range): $(cat /proc/sys/net/ipv4/ip_local_port_range)"
echo "• Время TIME_WAIT (/proc/sys/net/ipv4/tcp_fin_timeout): $(cat /proc/sys/net/ipv4/tcp_fin_timeout) сек"

# 3. Размеры буферов
echo -e "\n3. Размеры буферов:"
echo "• Макс. буфер чтения (/proc/sys/net/core/rmem_max): $(cat /proc/sys/net/core/rmem_max) байт"
echo "• Макс. буфер записи (/proc/sys/net/core/wmem_max): $(cat /proc/sys/net/core/wmem_max) байт"
echo "• TCP rmem (min,default,max): $(cat /proc/sys/net/ipv4/tcp_rmem)"
echo "• TCP wmem (min,default,max): $(cat /proc/sys/net/ipv4/tcp_wmem)"

# 4. Очереди соединений
echo -e "\n4. Очереди соединений:"
echo "• Макс. очередь подключений (/proc/sys/net/core/somaxconn): $(cat /proc/sys/net/core/somaxconn)"
echo "• SYN backlog (/proc/sys/net/ipv4/tcp_max_syn_backlog): $(cat /proc/sys/net/ipv4/tcp_max_syn_backlog)"

# 5. Firewall
echo -e "\n5. Состояние firewall:"
if command -v iptables &> /dev/null; then
    echo "• iptables правила:"
    iptables -L -n | grep -E "(DROP|REJECT|ACCEPT)" | head -n 5
else
    echo "• iptables не установлен"
fi

if command -v nft &> /dev/null; then
    echo -e "\n• nftables правила:"
    nft list ruleset | head -n 5
fi

# 6. SELinux/AppArmor
echo -e "\n6. Мандатный контроль доступа:"
if command -v sestatus &> /dev/null; then
    echo "• SELinux статус:"
    sestatus | grep "Current mode"
elif command -v aa-status &> /dev/null; then
    echo "• AppArmor статус:"
    aa-status | head -n 3
else
    echo "• Ни SELinux ни AppArmor не активны"
fi

# 7. Cgroup ограничения (если есть)
echo -e "\n7. Cgroup ограничения:"
if [ -d "/sys/fs/cgroup" ]; then
    echo "• memory.limit_in_bytes: $(cat /sys/fs/cgroup/memory/memory.limit_in_bytes 2>/dev/null || echo 'N/A')"
    echo "• cpu.cfs_quota_us: $(cat /sys/fs/cgroup/cpu/cpu.cfs_quota_us 2>/dev/null || echo 'N/A')"
else
    echo "• Cgroups не настроены"
fi

echo -e "\n=== Проверка открытых сокетов ==="
echo "• Всего TCP: $(ss -tH | wc -l)"
echo "• Всего UDP: $(ss -uH | wc -l)"
echo "• Всего UNIX: $(ss -xH | wc -l)"

echo -e "\n=== Проверка текущего процесса ==="
echo "• PID текущего shell: $$"
echo "• Лимиты процесса:"
grep -E "open files|Max processes" /proc/$$/limits

echo "Подробный список сокетов"
ss -tulnp