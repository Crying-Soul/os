#!/bin/bash

# Пути к исполняемым файлам и FIFO
CREATE_FIFO="./lb3/6/6.2/create_fifo"
WRITER="./lb3/6/6.2/writer"
READER="./lb3/6/6.2/reader"
ANALYZE_FIFO="./lb3/6/6.2/analyze_fifo"
CMD_PIPE="cmd_pipe"
PROG_PIPE="prog_pipe"

# Проверка существования необходимых файлов
required_files=("$CREATE_FIFO" "$WRITER" "$READER" "$ANALYZE_FIFO")
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

# 2. Создание FIFO разными способами
section "Создание FIFO"
echo "Создаем FIFO из командной строки: mkfifo $CMD_PIPE"
mkfifo "$CMD_PIPE"
ls -l "$CMD_PIPE"

echo -e "\nСоздаем FIFO программно:"
"$CREATE_FIFO"
ls -l "$PROG_PIPE"

# 3. Анализ FIFO
section "Анализ FIFO"

# 3.1. Утилитами ОС
echo -e "\nАнализ утилитами ОС:"
echo "file $CMD_PIPE: $(file "$CMD_PIPE")"
echo "stat $CMD_PIPE: $(stat -c '%A %i %s' "$CMD_PIPE")"
echo "ls -li $CMD_PIPE:"
ls -li "$CMD_PIPE"

# 3.2. Через /proc
inode=$(stat -c '%i' "$CMD_PIPE")
echo -e "\nПоиск в /proc по inode $inode:"
find /proc -inum "$inode" 2>/dev/null || echo "Не используется процессами"

# 3.3. Программный анализ
echo -e "\nПрограммный анализ:"
"$ANALYZE_FIFO"

# 4. Демонстрация работы FIFO
section "Демонстрация работы FIFO"
echo "Запускаем reader в фоне:"
"$READER" &
reader_pid=$!
sleep 1

echo -e "\nЗапускаем writer:"
"$WRITER"

wait "$reader_pid"

