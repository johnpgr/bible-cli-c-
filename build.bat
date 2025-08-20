@echo off
setlocal

set "CC=clang++"
set "CC_FLAGS=-std=c++23 -g -Wall -Wextra -Wpedantic -Wno-writable-strings"

REM ensure build directory exists
if not exist "build" (
    mkdir "build"
)

echo Compiling application

REM check compiler exists
where %CC% >nul 2>&1
if errorlevel 1 (
    echo %CC% not found in PATH.
    exit /b 1
)

"%CC%" %CC_FLAGS% -o "build\bible.exe" "src\main.cpp"
if errorlevel 1 (
    echo Compilation failed.
    exit /b %errorlevel%
)

echo Compiled successfully
echo Executable: build\bible.exe

endlocal
exit /b 0
