@echo off
setlocal
cd /d %~dp0

set VCVARS="C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars32.bat"
call %VCVARS% >nul
if errorlevel 1 (
    echo Failed to init MSVC x86 environment
    exit /b 1
)

if not exist build mkdir build

cl /nologo /LD /MT /W3 /O2 ^
    src\main.cpp src\layout.cpp src\log.cpp src\bgswitch.cpp ^
    /Fo:build\ /Fe:build\GameUI.dll ^
    /link /OUT:build\GameUI.dll user32.lib advapi32.lib

if errorlevel 1 (
    echo Build failed, not deploying.
    exit /b 1
)

set DEPLOY_TARGET=..\..\..\..\valve\cl_dlls\GameUI.dll
copy /Y build\GameUI.dll %DEPLOY_TARGET% >nul
if errorlevel 1 (
    echo Build OK, but deploy to %DEPLOY_TARGET% failed -- is the game running?
) else (
    echo Build OK, deployed to %DEPLOY_TARGET%.
)

endlocal
