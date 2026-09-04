@echo off
echo =======================================================
echo GitHub Repository Setup for C Line Editor
echo =======================================================
echo.

set /p REPO_URL="Enter your GitHub repository URL (e.g., https://github.com/your-username/c-line-editor.git): "

if "%REPO_URL%"=="" (
    echo [ERROR] Repository URL cannot be empty.
    pause
    exit /b
)

echo Initializing git repository...
git init
git add .
git commit -m "feat: complete C line editor with core features, bonus features, paper design, and docs"
git branch -M main
git remote add origin %REPO_URL%
echo.
echo Pushing to GitHub...
git push -u origin main

if %ERRORLEVEL% EQU 0 (
    echo.
    echo =======================================================
    echo [SUCCESS] Code pushed successfully to %REPO_URL%!
    echo =======================================================
) else (
    echo.
    echo [NOTE] If push failed due to remote already existing or branch differences:
    echo Try running: git push -u origin main --force
)
pause
