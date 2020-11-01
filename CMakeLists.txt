cmake_minimum_required(VERSION 3.10)
project(Sudoku C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

add_library(Stack STATIC src/Stack.c)
target_include_directories(Stack PRIVATE include)

add_executable(${PROJECT_NAME} src/sudoku.c)
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PUBLIC Stack)
