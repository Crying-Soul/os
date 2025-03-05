#!/bin/bash

cd 2.4

files=("situation_a.c" "situation_b.c" "situation_c.c")
#files=()
rm output.txt
for file in "${files[@]}"
do
    output="${file%.*}"
    gcc "$file" -o "$output"

    echo "------------------------" >> output.txt
    ./$output >> output.txt
    echo "------------------------" >> output.txt
    rm $output
done
