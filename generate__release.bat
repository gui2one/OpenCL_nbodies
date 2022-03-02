@echo off
mkdir build/Release
cmake . -G "MinGW Makefiles" -B ./build/Release -D CMAKE_BUILD_TYPE=Release