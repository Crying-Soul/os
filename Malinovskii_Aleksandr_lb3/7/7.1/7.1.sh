#!/bin/bash

# Компиляция
gcc server.c -o server && gcc client.c -o client
chmod +x server client

# Запуск сервера в фоне с перенаправлением вывода
./server > server.log 2>&1 &
SERVER_PID=$!
echo "Сервер запущен (PID: $SERVER_PID)"

# Ждем запуска сервера
sleep 2

# Запуск клиента
echo -e "\nЗапуск клиента..."
./client > client.log 2>&1

# Вывод логов
echo -e "\n=== Вывод сервера ==="
cat server.log

echo -e "\n=== Вывод клиента ==="
cat client.log

# Очистка
kill $SERVER_PID 2>/dev/null
rm server client server.log client.log