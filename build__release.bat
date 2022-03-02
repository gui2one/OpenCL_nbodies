@echo off
CALL ./generate__release.bat

cd build/Release
rm -f app.exe
mingw32-make -j 8

app.exe