@echo off
echo ======================================================================
echo  Building Intelligent Examination Scheduling System (C / MinGW GCC)
echo ======================================================================

set CC=C:\MinGW\bin\gcc.exe
if not exist %CC% (
    set CC=gcc
)

echo Using Compiler: %CC%

if not exist bin mkdir bin

%CC% -Wall -Wextra -std=c99 -O2 -Iinclude ^
    src\graph.c ^
    src\heap.c ^
    src\coloring.c ^
    src\room_allocation.c ^
    src\validator.c ^
    src\test_suite.c ^
    src\main.c ^
    -o bin\exam_scheduler.exe

if %ERRORLEVEL% equ 0 (
    echo [BUILD SUCCESS] Executable created at: bin\exam_scheduler.exe
) else (
    echo [BUILD ERROR] Compilation failed with exit code %ERRORLEVEL%
)
