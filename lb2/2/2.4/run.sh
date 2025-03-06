#!/bin/bash

files=("2.4-a.c" "2.4-b.c" "2.4-c.c")
#files=("situation_a.c")
rm output.txt
for file in "${files[@]}"
do
    output="${file%.*}"
    gcc "$file" -o "$output"

    echo "------------------------" >> output.txt
    ./$output >> output.txt
    ps -o pid,ppid,pgid,sid,comm -H f >>output.txt
    
    echo "------------------------" >> output.txt
    rm $output
done
