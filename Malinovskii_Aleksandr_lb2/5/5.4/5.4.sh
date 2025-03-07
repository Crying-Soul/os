#!/bin/bash

cd 5/5.4
files=("5.4.c" "quant.c")
for file in "${files[@]}"
do
    output="${file%.*}"
    gcc "$file" -o "$output"
done

echo Определение величины кванта
sudo ./quant
echo Меняем политику планирования на RR
sudo taskset -c 0 ./5.4

for file in "${files[@]}"
do
    output="${file%.*}"
    rm $output

done
