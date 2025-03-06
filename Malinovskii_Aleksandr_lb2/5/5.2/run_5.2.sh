#!/bin/bash

files=("5.2.c")
for file in "${files[@]}"
do
    output="${file%.*}"
    gcc "$file" -o "$output"
done

sudo taskset -c 0 ./5.2

for file in "${files[@]}"
do
    output="${file%.*}"
    rm $output

done
