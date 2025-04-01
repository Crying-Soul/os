#!/bin/bash

# Компиляция
gcc server.c -o udp_server
gcc client.c -o udp_client
chmod +x udp_server udp_client

# Запуск сервера в фоне
./udp_server &
SERVER_PID=$!
echo "UDP сервер запущен (PID: $SERVER_PID)"

# Даем серверу время на запуск
sleep 2

# Запуск клиента
echo -e "\nЗапуск UDP клиента..."
./udp_client


# Остановка сервера и очистка
kill $SERVER_PID 2>/dev/null
rm udp_server udp_client 