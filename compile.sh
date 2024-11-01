#!/bin/bash

# Ensure the script is executable
# Run chmod +x compile_and_run.sh before executing

# Compile the project
g++ -std=c++11  main.c db.c -o db_main

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful!"
    
    # Run the executable with a database file
    ./db_main mydb.db
else
    echo "Compilation failed!"
    exit 1
fi