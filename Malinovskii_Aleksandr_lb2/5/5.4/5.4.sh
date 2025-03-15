#!/bin/bash

cd 5/5.4/
files=("quant.c")
for file in "${files[@]}"
do
    output="${file%.*}"
    gcc "$file" -o "$output"
done

echo Определение величины кванта
echo =================
sudo ./quant
echo =================
echo Меняем политику планирования на RR

cd 5.4.1
echo -e "\n==========================5.4.1=================================="
echo  -e "Запуск процессов с одинаковыми приоритетами\n"
sudo taskset -c 0 ./5.4.1.sh

cd ..
sleep 1
cd 5.4.2
echo -e "\n==========================5.4.2=================================="
echo  -e "Запуск процессов с разными приоритетами\n"
sudo taskset -c 0 ./5.4.2.sh

cd ..
sleep 1
cd 5.4.3
echo -e "\n==========================5.4.3=================================="
echo  -e "Пример использование yield\n"
sudo taskset -c 0 ./5.4.3.sh

cd ..

for file in "${files[@]}"
do
    output="${file%.*}"
    rm $output

done
