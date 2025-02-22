#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: $0 <output_file>"
    exit 1
fi

output_file="$1"
> "$output_file"

find_type() {
    local type="$1"
    local found=0
    sudo ls -lR / 2>/dev/null | awk -v type="$type" -v found="$found" '
    /:$/ {
        sub(/:$/, "", $0)
        dir = $0
        next
    }
    $1 ~ "^"type {
        if (dir == "/")
            printf "%s %s\n", $1, dir $NF;
        else
            printf "%s %s/%s\n", $1, dir, $NF;
        exit
    }
' >> "$output_file"
}


for type in "-" "b" "c" "d" "l" "p" "s"; do
    find_type "$type"
done

echo "Results saved in '$output_file'."
