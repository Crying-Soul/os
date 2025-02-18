#!/bin/bash

# Путь к файлу и директории
TARGET_FILE="file.txt"

# Создаем символьную ссылку с помощью ln -s
ln -s "$TARGET_FILE" "./link_ln.txt"

# Создаем жесткую ссылку с помощью link 
link "$TARGET_FILE" "./link_link.txt"

# Создаем символьную ссылку с помощью cp -s
cp -s "$TARGET_FILE" "./link_cp.txt"

# Проверяем созданные ссылки
echo "Проверка созданных ссылок:"
ls -l 