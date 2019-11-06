cmake_minimum_required(VERSION 3.10)

# set the project name
project(iv)

# CMake arcana

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

find_package(Threads)

add_executable(iv main.cpp)

# More CMake arcana
# Specify libraries or flags to use when linking a given target and/or its dependents.
target_link_libraries(iv Threads::Threads)
