:: TODO(me): this bash script needs some rewriting
:: TODO(me): ADD A COMMAND FOR ADDRESS SANITISATION, LOOK AT RADDBG
@echo off
setlocal
:: Parse command line arguments
if "%1"=="main" set main=1
if "%1"=="test" set test=1
if "%1"=="help" goto :help
if "%1"=="-h" goto :help
if "%1"=="?" goto :help

if not exist build mkdir build

:: Compiler flags with include paths
set cl_common=/I../src /nologo /FC /Zi /EHsc /std:c++20 /MD 
set link=/MACHINE:X64 /LIBPATH:../src/third_party/raylib/lib raylib.lib opengl32.lib kernel32.lib user32.lib shell32.lib gdi32.lib winmm.lib msvcrt.lib 
set compile=call cl %cl_common%


:: Build targets
if "%test%"=="1" (
    set target=../src/tester/tester_main.cpp
    set output=test.exe
    echo Building tests
) else if "%main%"=="1" (
    set target=../src/entry/main.cpp
    set output=main.exe
    echo Building main program
) else (
    set target=../src/entry/main.cpp
    set output=main.exe
    echo Building main program [default]
)
pushd build
echo Compiling: %target%
%compile% %target% /Fe:%output% /link %link%

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    popd
    exit /b %ERRORLEVEL%
)

echo Build successful: %output%
popd
goto :end

:help
echo Usage: build.bat [target]
echo.
echo Targets:
echo   test    - Build test executable
echo   main    - Build main executable
echo   (none)  - Build main executable (default)
echo.
echo Examples:
echo   build.bat test
echo   build.bat main
echo   build.bat

:end
