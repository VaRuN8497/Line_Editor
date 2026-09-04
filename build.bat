@echo off
echo ===========================================
echo Compiling C Line Editor...
echo ===========================================
gcc -Wall -Wextra -O2 -std=c99 editor.c -o editor.exe
if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] editor.exe compiled successfully!
    echo Run it with: editor.exe
) else (
    echo [ERROR] Compilation failed. Ensure GCC / MinGW is installed in PATH.
)
pause
