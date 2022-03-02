@echo off
CALL ./generate__debug.bat

cd build/Debug
rm -f app.exe
mingw32-make -j 8

app.exe