@echo off

cmake -S "%~dp0." -B "%~dp0build\vs2022" -G "Visual Studio 17 2022" -A x64
if errorlevel 1 goto :failed

echo.
echo Solution generated: build\vs2022\bomberman.sln
start "" "%~dp0build\vs2022\bomberman.sln"
exit /b 0

:failed
echo.
echo CMake failed. Is CMake installed and on PATH? Visual Studio ships it under
echo   C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin
pause
exit /b 1
