cmake_minimum_required(VERSION 3.23)
project(twotapecmake)

set(CMAKE_CXX_STANDARD 14)

add_executable(tm_interpreter tm_interpreter.cpp turing_machine.cpp turing_machine.h)