#!/bin/bash

files=("son1.c" "son2.c" "son3.c" "son4.c" "son5.c" "father.c")
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
