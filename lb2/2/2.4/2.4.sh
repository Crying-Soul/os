#!/bin/bash

files=("lb2/2/2.4/2.4-a" "lb2/2/2.4/2.4-b" "lb2/2/2.4/2.4-c")
for file in "${files[@]}"
do
    echo "------------------------" >> out.txt
    echo "Запуск программы: $file" >> out.txt

    # Проверяем, существует ли файл
    if [[ -f "./$file" ]]; then
        ./$file >> out.txt  # Запускаем программу и записываем вывод в out.txt
    else
        echo "Файл $file не найден." >> out.txt
        continue
    fi

    # Вывод информации о процессах
    echo "Состояние процессов после запуска $file:" >> out.txt
    ps -o pid,ppid,pgid,sid,comm -H f >> out.txt

    # Если это 2.4-b, выполняем дополнительные действия
    if [[ "$file" == "lb2/2/2.4/2.4-b" ]]; then
        echo "Дополнительная информация для 2.4-b:" >> out.txt

        # Ищем процесс 2.4-b в выводе ps и извлекаем его PPID
        ppid=$(ps -o pid,ppid,comm -H f | grep "2.4-b" | awk '{print $2}')
        if [[ -n "$ppid" ]]; then
            echo "Родительский PID для 2.4-b: $ppid" >> out.txt
            echo "Процессы, связанные с PPID $ppid:" >> out.txt
            ps aux | grep "$ppid" | grep -v grep >> out.txt
        else
            echo "Процесс 2.4-b не найден." >> out.txt
        fi
    fi

    echo "------------------------" >> out.txt
done