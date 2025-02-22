#!/bin/bash

# Функция для логирования с временной меткой
log() {
    local timestamp
    timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    echo -e "\n[$timestamp] $*"
}

# Функция для обработки ошибок
handle_error() {
    log "Ошибка: $1"
    exit 1
}

# Проверка наличия пользователей и создание, если не существуют
create_user() {
    if ! id "$1" &>/dev/null; then
        log "sudo useradd -m $1"
        sudo useradd -m "$1" || handle_error "Не удалось создать пользователя $1"
    else
        log "Пользователь '$1' уже существует."
    fi
}

# Проверка наличия группы и создание, если не существует
create_group() {
    if ! getent group "$1" &>/dev/null; then
        log "sudo groupadd $1"
        sudo groupadd "$1" || handle_error "Не удалось создать группу $1"
    else
        log "Группа '$1' уже существует."
    fi
}

# Создание пользователей
create_user "user1"
create_user "user2"
create_user "user3"

# Создание группы и добавление пользователей в неё
create_group "testgroup"
log "sudo usermod -aG testgroup user1"
sudo usermod -aG testgroup user1 || handle_error "Не удалось добавить пользователя 'user1' в группу 'testgroup'"
log "sudo usermod -aG testgroup user2"
sudo usermod -aG testgroup user2 || handle_error "Не удалось добавить пользователя 'user2' в группу 'testgroup'"

# Проверяем текущие группы пользователей
log "--- Текущие группы пользователей ---"
log "groups user1"
groups user1
log "groups user2"
groups user2
log "-----------------------------------"

# Создание и настройка тестового файла
log "--- Создание и настройка тестового файла 'secret.txt' ---"
log "echo 'Secret data' | sudo tee ./secret.txt"
echo "Secret data" | sudo tee ./secret.txt > /dev/null || handle_error "Не удалось создать файл secret.txt"
log "sudo chown user1:testgroup ./secret.txt"
sudo chown user1:testgroup ./secret.txt || handle_error "Не удалось изменить владельца файла secret.txt"
log "sudo chmod 000 ./secret.txt"
sudo chmod 000 ./secret.txt || handle_error "Не удалось изменить права доступа для файла secret.txt"

# Проверяем текущие права файла
log "Текущие права доступа для файла 'secret.txt':"
log "ls -l ./secret.txt"
ls -l ./secret.txt
log "-----------------------------------"

# Компиляция программы-шлюза
log "--- Компиляция программы 'file_gateway' ---"
log "gcc -o file_gateway ./lb1/gateway.c"
gcc -o file_gateway ./lb1/gateway.c || handle_error "Не удалось скомпилировать gateway.c"
log "-----------------------------------"

# Установка SUID бита на программу-шлюз
log "--- Установка SUID на программу 'file_gateway' ---"
log "sudo chown root:root file_gateway"
sudo chown root:root file_gateway || handle_error "Не удалось изменить владельца программы file_gateway"
log "sudo chmod u+s file_gateway"
sudo chmod u+s file_gateway || handle_error "Не удалось установить SUID бит на программу file_gateway"

# Проверяем права на программу-шлюз
log "Текущие права доступа для программы 'file_gateway':"
log "ls -l file_gateway"
ls -l file_gateway
log "-----------------------------------"

# Функция для проверки доступа к файлу
check_access() {
    local user=$1
    local file=$2
    log "Проверка доступа для пользователя '$user' к файлу '$file'..."
    log "sudo -u $user ./file_gateway $file"
    sudo -u "$user" ./file_gateway "$file" || handle_error "Проверка доступа для пользователя '$user' не удалась"
    log "Доступ проверен для пользователя '$user'."
}

# Проверка доступа для пользователей
check_access "user1" "./secret.txt"
check_access "user2" "./secret.txt"

# Проверка доступа для пользователя, не входящего в группу
check_access "user3" "./secret.txt"

log "--- Скрипт завершён успешно! ---"
