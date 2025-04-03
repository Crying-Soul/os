#!/bin/bash

# Запуск сервера в фоне
strace -e trace=network ./lb3/7/7.2/server &
SERVER_PID=$!
echo "UDP сервер запущен (PID: $SERVER_PID)"

# Даем серверу время на запуск
sleep 2

# Запуск клиента
echo -e "\nЗапуск UDP клиента..."
strace -e trace=network ./lb3/7/7.2/client


# Остановка сервера и очистка
kill $SERVER_PID 2>/dev/null
