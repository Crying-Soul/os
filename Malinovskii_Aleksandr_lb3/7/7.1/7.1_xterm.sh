#!/bin/bash

# Файлы программ
SERVER_SRC="server.c"
CLIENT_SRC="client.c"
SERVER_BIN="server"
CLIENT_BIN="client"

# Функция для компиляции
compile_programs() {
    echo "Компиляция сервера..."
    if ! gcc "$SERVER_SRC" -o "$SERVER_BIN"; then
        echo "Ошибка компиляции сервера!"
        exit 1
    fi
    
    echo "Компиляция клиента..."
    if ! gcc "$CLIENT_SRC" -o "$CLIENT_BIN"; then
        echo "Ошибка компиляции клиента!"
        exit 1
    fi
    
    echo "Компиляция завершена успешно."
    chmod +x "$SERVER_BIN" "$CLIENT_BIN"
}

# Функция для запуска программ
run_programs() {
    # Проверяем наличие xterm
    if ! command -v xterm &> /dev/null; then
        echo "Ошибка: xterm не установлен. Установите его или модифицируйте скрипт для использования другого терминала."
        exit 1
    fi
    
    # Запускаем сервер в одном терминале
    xterm -hold -title "TCP Сервер" -e "./$SERVER_BIN" &
    SERVER_PID=$!
    echo "Сервер запущен (PID: $SERVER_PID)"
    
    # Даем серверу время на запуск
    sleep 2
    
    # Запускаем клиент в другом терминале
    xterm -hold -title "TCP Клиент" -e "./$CLIENT_BIN" &
    CLIENT_PID=$!
    echo "Клиент запущен (PID: $CLIENT_PID)"
    
    # Ждем завершения клиента
    wait $CLIENT_PID
    
    # Даем серверу время обработать соединение
    sleep 1
    
    # Завершаем сервер
    kill $SERVER_PID 2>/dev/null
    echo "Сервер остановлен."
}

# Основной код
clear
echo "=== Демонстрация TCP-сокетов на C ==="

# Компилируем программы
compile_programs

# Запускаем программы
echo -e "\nЗапуск сервера и клиента..."
run_programs

echo -e "\nДемонстрация завершена."
exit 0