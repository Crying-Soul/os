#!/bin/bash

log() {
    local timestamp
    timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    echo -e "\n[$timestamp] $*"
}

# log "Удаление старых файлов: sudo rm -f /usr/share/misc/magic-custom"
if [ -f "/usr/share/misc/magic-custom" ]; then
    sudo rm -f "/usr/share/misc/magic-custom"
fi

# log "Удаление старого тестового файла: rm -f example.mytype"
if [ -f "example.mytype" ]; then
    rm -f "example.mytype"
fi

CUSTOM_MAGIC_FILE="/usr/share/misc/magic-custom"
log "sudo tee $CUSTOM_MAGIC_FILE"
sudo tee "$CUSTOM_MAGIC_FILE" > /dev/null <<EOL
# My Custom File Type
0 string \x90MYT\x0D\x0A\x1A\x0A My Custom File Type
EOL

if [ -f "$CUSTOM_MAGIC_FILE" ]; then
    log "sudo file -C -m $CUSTOM_MAGIC_FILE"
    sudo file -C -m "$CUSTOM_MAGIC_FILE"
else
    echo "Ошибка: файл $CUSTOM_MAGIC_FILE не найден."
    exit 1
fi

log "sed -i 's|export MAGIC=.*|export MAGIC=/usr/share/misc/magic:/usr/share/misc/magic-custom|' ~/.bashrc"
if grep -q "export MAGIC=" ~/.bashrc; then
    sed -i 's|export MAGIC=.*|export MAGIC=/usr/share/misc/magic:/usr/share/misc/magic-custom|' ~/.bashrc
else
    log "echo 'export MAGIC=/usr/share/misc/magic:/usr/share/misc/magic-custom' >> ~/.bashrc"
    echo 'export MAGIC=/usr/share/misc/magic:/usr/share/misc/magic-custom' >> ~/.bashrc
fi
source ~/.bashrc

TEST_FILE="example.mytype"
log "echo -e '\\x90MYT\\x0D\\x0A\\x1A\\x0A' > $TEST_FILE"
echo -e "\x90MYT\x0D\x0A\x1A\x0A" > "$TEST_FILE"

log "file $TEST_FILE"
file "$TEST_FILE"

log "Скрипт завершен."
