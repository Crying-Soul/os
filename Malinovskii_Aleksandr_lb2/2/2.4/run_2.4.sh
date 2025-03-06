#!/bin/bash

files=("situation_a.c" "situation_b.c" "situation_c.c")
#files=("situation_a.c")
rm -f output.txt  # Удаляем старый файл output.txt, если он существует

for file in "${files[@]}"
do
    output="${file%.*}"  # Убираем расширение .c для получения имени выходного файла
    gcc "$file" -o "$output"  # Компилируем программу

    echo "------------------------" >> output.txt
    echo "Запуск программы: $output" >> output.txt
    ./$output >> output.txt  # Запускаем программу и записываем вывод в output.txt

    # Вывод информации о процессах
    echo "Состояние процессов после запуска $output:" >> output.txt

    # Если это situation_b, выполняем дополнительные действия
    if [[ "$output" == "situation_b" ]]; then
        ps -o pid,ppid,pgid,sid,comm -H f >> output.txt
        echo "Дополнительная информация для situation_b:" >> output.txt

        # Ищем процесс situation_b в выводе ps и извлекаем его PPID
        ppid=$(ps -o pid,ppid,comm -H f | grep "situation_b" | awk '{print $2}')
        pid=$(ps -o pid,ppid,comm -H f | grep "situation_b" | awk '{print $2}')
        if [[ -n "$ppid" ]]; then
            echo "Родительский PID для situation_b: $ppid" >> output.txt
            echo "Процессы, связанные с PPID $ppid:" >> output.txt
            ps aux | grep "$ppid" >> output.txt
        else
            echo "Процесс situation_b не найден." >> output.txt
        fi
    fi

    echo "------------------------" >> output.txt
    rm -f $output  # Удаляем скомпилированный файл
done