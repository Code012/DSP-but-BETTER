:: TODO(me): this bash script needs some rewriting
:: TODO(me): ADD A COMMAND FOR ADDRESS SANITISATION, LOOK AT RADDBG
@echo off
setlocal
:: Parse command line arguments
if "%1"=="main" set main=1
if "%1"=="test" set test=1
if "%1"=="clay" set clay=1
if "%1"=="help" goto :help
if "%1"=="-h" goto :help
if "%1"=="?" goto :help

if not exist build mkdir build

:: Compiler flags with include paths
:: if you want to link crt statically, build raylib from source with static crt first or find if they have done so
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
) else if "%clay%"=="1" (
    set target=../src/clay_examples/main.c
    set output=main.exe
    echo Building clay example program
) else (
    set target=../src/entry/main.cpp
    set output=main.exe
    echo Building main program [default]
)
pushd build

set main_code_path =..src/entry
set test_code_path =..src/tester
set raylib_lib_path=..src/third_party/raylib/lib
:: GENERAL COMPILER FLAGS
set compiler=               -nologo &:: Suppress Startup Banner
set compiler=%compiler%     -Oi     &:: Use assembly intrinsics where possible
set compiler=%compiler%     -MT     &:: Include CRT library in the executable (static link), Don't rely on the user having the correct CRT dll version
set compiler=%compiler%     -Gm-    &:: Disable minimal rebuild
set compiler=%compiler%     -GR-    &:: Disable runtime type info (C++)
set compiler=%compiler%     -EHa-   &:: Disable exception handling (C++)
set compiler=%compiler%     -W4     &:: So windows warnings go away
set compiler=%compiler%     -WX     &:: Treat all warnings as errors
:: IGNORE WARNINGS
set compiler=%compiler%     -wd4201 &:: Nameless struct/union
set compiler=%compiler%     -wd4100 &:: Unused function parameter
set compiler=%compiler%     -wd4189 &:: Local variable not referenced
set compiler=%compiler%     -wd4701 &:: Potentially uninitialized local variable 'name' used
set compiler=%compiler% 
:: DEBUG VARIABLES
set debug=        -FC &:: Produce the full path of the source code file
set debug=%debug% -Z7 &:: Produce debug information
:: WIN32 LINKER SWITCHES
set win32_link=             -subsystem:windows,5.2  &:: subsystem, 5.1 for x86
set win32_link=%win32_link% -opt:ref                &:: Remove unused functions
:: WIN32 PLATFORM LIBRARIES
set win32_libs=             user32.lib
set win32_libs=%win32_libs% kernel32.lib
set win32_libs=%win32_libs% shell32.lib
set win32_libs=%win32_libs% Gdi32.lib
set win32_libs=%win32_libs% Winmm.lib
set win32_libs=%win32_libs% opengl32.lib
:: THIRD PARTY LIBRARIES
set third_party_libs= raylib.lib

echo Compiling: %target%
%compile% %target% -DBUILD_DEBUG=1 /Fe:%output% /link %link%



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
echo   clay    - Build clay example
echo   (none)  - Build main executable (default)
echo.
echo Examples:
echo   build.bat test
echo   build.bat main
echo   build.bat clay
echo   build.bat

:end
