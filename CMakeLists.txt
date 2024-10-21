cmake_minimum_required(VERSION 3.10)
project(DatabaseProject C CXX)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set C++ standard (for tests)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add the include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Create a library for the database functions
add_library(db_lib db.c)

# Add the executable for the main program
add_executable(db_main main.c)
target_link_libraries(db_main db_lib)

# Add the executable for the test
add_executable(db_test db_test.cpp)

# Link the test executable with Google Test and the database library
target_link_libraries(db_test gtest_main db_lib)

enable_testing()
add_test(NAME db_test COMMAND db_test)