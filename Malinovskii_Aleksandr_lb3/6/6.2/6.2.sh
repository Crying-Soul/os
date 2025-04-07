#!/bin/bash
# =============================================================================
# Скрипт для компиляции и анализа программы исследования свойств сигналов
# =============================================================================
# Цели:
# 1. Скомпилировать программу queue_signals.c
# 2. Запустить программу с трассировкой системных вызовов, связанных с сигналами
# 3. Проанализировать результаты для понимания приоритетов и организации очереди
#    для сигналов.
#
# Особенности реализации:
# - Используется strace для мониторинга работы с сигналами.
# - Анализ работы с сигналами второй половины списка (kill -l).
# - Проверка наличия приоритетов и FIFO для сигналов реального времени.
# - Изучение возможности организации очереди для различных типов сигналов.
#
# Выводы:
# 1. strace позволяет детально отслеживать взаимодействие программы с сигналами.
# 2. Анализ результатов трассировки позволяет определить приоритеты сигналов.
# 3. Изучение организации очереди для сигналов различных типов.
# 4. Подтверждение или


cd 6/6.2
echo "=============================================="
echo "                  ЗАДАНИЕ 6.2                 "
echo "=============================================="
# Проверка существования необходимых файлов
required_files=("create_fifo.c" "writer.c" "reader.c" "analyze_fifo.c")
missing_files=()

for file in "${required_files[@]}"; do
    if [ ! -f "$file" ]; then
        missing_files+=("$file")
    fi
done

if [ ${#missing_files[@]} -ne 0 ]; then
    echo "Ошибка: Отсутствуют следующие файлы:"
    printf '%s\n' "${missing_files[@]}"
    exit 1
fi

# Функция для вывода разделителя
section() {
    echo -e "\n=== $1 ==="
}

# 1. Компиляция программ
section "Компиляция программ"
gcc create_fifo.c -o create_fifo
gcc writer.c -o writer
gcc reader.c -o reader
gcc analyze_fifo.c -o analyze_fifo


# 2. Создание FIFO разными способами
section "Создание FIFO"
echo "Создаем FIFO из командной строки: mkfifo cmd_pipe"
mkfifo cmd_pipe
ls -l cmd_pipe

echo -e "\nСоздаем FIFO программно:"
./create_fifo
ls -l prog_pipe

# 3. Анализ FIFO
section "Анализ FIFO"

# 3.1. Утилитами ОС
echo -e "\nАнализ утилитами ОС:"
echo "file cmd_pipe: $(file cmd_pipe)"
echo "stat cmd_pipe: $(stat -c '%A %i %s' cmd_pipe)"
echo "ls -li cmd_pipe:"
ls -li cmd_pipe

# 3.3. Программный анализ
echo -e "\nПрограммный анализ:"
./analyze_fifo

# 4. Демонстрация работы FIFO
section "Демонстрация работы FIFO"
echo "Запускаем reader в фоне:"
strace -f -e write,read ./reader &
reader_pid=$!
sleep 1

echo -e "\nЗапускаем writer:"
strace -f -e trace=write ./writer

wait $reader_pid

rm -f cmd_pipe prog_pipe create_fifo writer reader analyze_fifo
