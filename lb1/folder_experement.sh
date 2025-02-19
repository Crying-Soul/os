#!/bin/bash

# Parameters
num_files=10        # Number of files to create
file_size=1M        # Size of each file
num_delete=5        # Number of files to delete

# Create a new directory
mkdir test_dir
cd test_dir

# Function to measure directory size
get_dir_size() {
    du -sb . | awk '{print $1}'  # Get size in bytes
}

# Measure initial directory size
initial_size=$(get_dir_size)
echo "Initial directory size: $initial_size bytes"

# Create files
for i in $(seq 1 $num_files); do
    dd if=/dev/zero of=file$i bs=$file_size count=1 > /dev/null 2>&1  # Create a file of specified size
    current_size=$(get_dir_size)
    echo "Added file$i. Directory size: $current_size bytes"
done

# Delete some files
for i in $(seq 1 $num_delete); do
    rm file$i  # Delete the file
    current_size=$(get_dir_size)
    echo "Deleted file$i. Directory size: $current_size bytes"
done

# Clean up: remove the directory
cd ..
rm -rf test_dir