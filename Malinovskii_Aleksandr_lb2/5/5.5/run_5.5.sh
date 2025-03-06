#!/bin/bash

files=("5.5.c")
for file in "${files[@]}"
do
    output="${file%.*}"
    gcc "$file" -o "$output"
done

echo Процессы созданные программно
taskset -c 0 ./5.5
for file in "${files[@]}"
do
    output="${file%.*}"
    rm $output

done
