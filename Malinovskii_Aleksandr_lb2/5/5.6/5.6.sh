#!/bin/bash

cd 5/5.6
files=("5.6.c")
for file in "${files[@]}"
do
    output="${file%.*}"
    gcc "$file" -o "$output"
done

taskset -c 0 ./5.6
for file in "${files[@]}"
do
    output="${file%.*}"
    rm $output

done
