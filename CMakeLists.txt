cmake_minimum_required(VERSION 3.18)

project(MATHEMATICAL_MORPHOLOGY LANGUAGES C CXX CUDA)

set(CMAKE_CUDA_ARCHITECTURES "native")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE SOURCES "main.c" "sequential/*.c" "parallel/*.c" "parallel/*.cu")

add_executable(main ${SOURCES}) 