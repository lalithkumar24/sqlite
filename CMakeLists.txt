cmake_minimum_required(VERSION 3.10)

# Set the project name
project(SimpleDB C)

# Add the source files
set(SOURCES
    db.c
)

# Create the executable
add_executable(simple_db ${SOURCES})

# Include directories for header files
target_include_directories(simple_db PRIVATE .)
