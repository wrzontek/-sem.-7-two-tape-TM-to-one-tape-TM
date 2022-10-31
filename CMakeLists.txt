cmake_minimum_required(VERSION 3.18.4)
project(twotapecmake)

set(CMAKE_CXX_STANDARD 14)

add_executable(tm_translator tm_translator.cpp turing_machine.cpp turing_machine.h)