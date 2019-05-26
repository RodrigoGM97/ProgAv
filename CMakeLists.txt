cmake_minimum_required(VERSION 3.13)
project(ProgAv C)

set(CMAKE_C_STANDARD 11)


add_executable(ProgAv "Proyecto Final/main.cpp")
target_link_libraries(ProgAv m)
target_compile_options(ProgAv PRIVATE -Wall -pedantic -fopenmp)

FIND_PACKAGE(OpenMP)
if(OPENMP_FOUND)
    message("OPENMP FOUND")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()
include_directories(SYSTEM ${OpenMP_INCLUDE_PATH})