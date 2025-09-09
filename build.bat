 @echo off
if not exist build mkdir build

:: Compiler flags with include paths
set cl_common=/I../src /nologo /FC /Z7
set compile=call cl %cl_common%

:: Build targets
if "%test%"=="1" set target=../src/test/test_main.cpp && set output=test.exe
if "%main%"=="1" set target=../src/entry/main.cpp && set output=main.exe
if "%target%"=="" set target=../src/entry/main.cpp && set output=main.exe

pushd build
%compile% ../src/entry/main.cpp /Fe:%output%
popd
