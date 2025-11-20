@echo off

:: Parse command line arguments
if "%1"=="test" set test=1
if "%1"=="main" set main=1
if "%1"=="help" goto :help
if "%1"=="-h" goto :help
if "%1"=="?" goto :help

:: Build targets
:: TODO: change targets for main to be .sln files when you get to it
if "%test%"=="1" (
    set target=build/test.exe
    set file=/src/tester/tester_main.cpp
    echo Debugging tests
) else if "%main%"=="1" (
    set target=build/main.exe
    echo Debugging main program
) else (
    set target=build/main.exe
    echo Debugging main program
)


start raddbg %target% --user:dsp-user.txt --auto_step



goto :end

:help
echo Usage: debug.bat [target]
echo.
echo Targets:
echo   test    - Debug test solution 
echo   main    - Debug main solution 
echo   (none)  - Debug main solution (default)
echo.
echo Examples:
echo   debug.bat test
echo   debug.bat main
echo   debug.bat

:end

