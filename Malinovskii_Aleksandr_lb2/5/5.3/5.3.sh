#!/bin/bash

cd 5/5.3
files=("5.3.1.c" "5.3.2.c" "5.3.3.c")
for file in "${files[@]}"
do
    output="${file%.*}"
    gcc "$file" -o "$output"
done


echo 5.3.1
sudo taskset -c 0 ./5.3.1
echo 5.3.2
sudo taskset -c 0 ./5.3.2
echo 5.3.3
sudo taskset -c 0 ./5.3.3

for file in "${files[@]}"
do
    output="${file%.*}"
    rm $output

done
