#!/bin/bash

cd 5/5.1
files=("5.1_prog.c" "5.1_table.c")
for file in "${files[@]}"
do
    output="${file%.*}"
    gcc "$file" -o "$output"
done

echo Процессы созданные программно
./5.1_prog
echo Процессы взятые из таблицы процессов
./5.1_table
for file in "${files[@]}"
do
    output="${file%.*}"
    rm $output

done
