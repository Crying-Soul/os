#!/bin/bash

cd 3/3.1
files=("son1.c" "son2.c" "son3.c" "father.c")
for file in "${files[@]}"
do
    output="${file%.*}"
    gcc "$file" -o "$output"
done

./father

for file in "${files[@]}"
do
    output="${file%.*}"
    rm $output
done
cat processes_after.txt
rm processes_after.txt
