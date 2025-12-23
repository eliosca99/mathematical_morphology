cmake_minimum_required(VERSION 3.10)

project(MATHEMATICAL_MORPHOLOGY)

file(GLOB_RECURSE SOURCES "main.c" "sequential/*.c" "parallel/*.c")

add_executable(main ${SOURCES}) 