#!/bin/bash

cd 5/5.6
files=("5.6.1.c" "5.6.2.c")
for file in "${files[@]}"
do
    output="${file%.*}"
    gcc "$file" -o "$output"
done

echo -e "\n5.6.1\n"
taskset -c 0 ./5.6.1
echo -e "\n5.6.2\n"
taskset -c 0 ./5.6.2
for file in "${files[@]}"
do
    output="${file%.*}"
    rm $output

done
