#!/bin/bash

# 1. Compile the project first to ensure we are running the latest code
echo "Building project..."
make
if [ $? -ne 0 ]; then
    echo "Build failed. Exiting."
    exit 1
fi

# 2. Create the output directory if it doesn't exist
mkdir -p outputs

echo "----------------------------------------"
echo "Running Tests..."
echo "----------------------------------------"

# 3. Loop through all .txt files in the tests directory
for input_file in tests/*.txt; do
    # Check if files actually exist
    [ -e "$input_file" ] || continue

    # Extract the filename without extension (e.g., "test_allocator")
    base_name=$(basename "$input_file" .txt)
    
    # Define the output filename
    output_file="outputs/${base_name}.out"

    echo "Running $base_name..."
    
    # Run the simulator with the input file and redirect stdout to the output file
    # Note: We pass the input file as an argument as implemented in main.cpp
    ./memsim "$input_file" > "$output_file"
done

echo "----------------------------------------"
echo "All tests completed."
echo "Results stored in 'outputs/' directory."