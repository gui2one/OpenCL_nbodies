@echo off
mkdir build\Release
cmake . -G "MinGW Makefiles" -B ./build/Release -DCMAKE_BUILD_TYPE=Release