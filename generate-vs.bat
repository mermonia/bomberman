@echo off

rem No -G on purpose: CMake picks the newest Visual Studio it finds, so this
rem works on 2022 and 2026 alike. The presets in CMakePresets.json pin a
rem specific version when you want one.
cmake -S "%~dp0." -B "%~dp0build\vs" -A x64
if errorlevel 1 goto :failed

rem VS 2026 writes the new .slnx format; older versions write .sln.
set "SLN=%~dp0build\vs\bomberman.slnx"
if not exist "%SLN%" set "SLN=%~dp0build\vs\bomberman.sln"
if not exist "%SLN%" goto :nosln

echo.
echo Solution generated: %SLN%
start "" "%SLN%"
exit /b 0

:nosln
echo.
echo CMake reported success but no solution was found in build\vs.
pause
exit /b 1

:failed
echo.
echo CMake failed. Is CMake installed and on PATH? Visual Studio ships it under
echo   <VS install>\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin
pause
exit /b 1
