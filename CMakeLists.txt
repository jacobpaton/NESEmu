cmake_minimum_required(VERSION 3.18.1)
project(NESEmu)
include_directories(./include)
add_executable(NESEmu ./src/nes.cpp ./src/cpu.cpp ./src/ppu.cpp ./src/main.cpp)
