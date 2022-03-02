@echo off
mkdir build/Debug
cmake . -G "MinGW Makefiles" -B ./build/Debug -D CMAKE_BUILD_TYPE=Debug