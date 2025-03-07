#!/bin/bash

cd 5/5.5
files=("5.5.c")
for file in "${files[@]}"
do
    output="${file%.*}"
    gcc "$file" -o "$output"
done

taskset -c 0 ./5.5
for file in "${files[@]}"
do
    output="${file%.*}"
    rm $output

done
