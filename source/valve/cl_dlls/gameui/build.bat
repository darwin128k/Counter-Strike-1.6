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
    src\main.cpp src\layout.cpp src\log.cpp src\bgswitch.cpp src\roundframe.cpp ^
    /Fo:build\ /Fe:build\vellum.dll ^
    /link /OUT:build\vellum.dll /EXPORT:Vellum_Init user32.lib advapi32.lib

if errorlevel 1 (
    echo Build failed, not deploying.
    exit /b 1
)

set ROOT=..\..\..\..\
set CL_DLLS=%ROOT%valve\cl_dlls
set ORIG_GAMEUI=%CL_DLLS%\GameUI_orig.dll
set STEAM_ORIG=%~dp0orig\steam_api.dll
set STEAM_DST=%ROOT%steam_api.dll

if not exist "%ORIG_GAMEUI%" (
    echo Missing original GameUI: %ORIG_GAMEUI%
    exit /b 1
)

if not exist "%STEAM_ORIG%" (
    echo Missing pristine steam_api: %STEAM_ORIG%
    echo Put the unpatched steam_api.dll in source\valve\cl_dlls\gameui\orig\
    exit /b 1
)

copy /Y build\vellum.dll "%ROOT%vellum.dll" >nul
if errorlevel 1 (
    echo Built OK, but could not copy vellum.dll to game root -- is the game running?
    exit /b 1
)
if exist "%ROOT%GameUI_hook.dll" del /Q "%ROOT%GameUI_hook.dll"

copy /Y "%ORIG_GAMEUI%" "%CL_DLLS%\GameUI.dll" >nul
if errorlevel 1 (
    echo Could not restore original GameUI.dll -- is the game running?
    exit /b 1
)

python patch_imports.py "%STEAM_ORIG%" "%STEAM_DST%" --dll vellum.dll --func Vellum_Init
if errorlevel 1 (
    echo PE patch of steam_api.dll failed.
    exit /b 1
)

echo Build OK: original GameUI.dll restored, vellum.dll next to hl.exe, steam_api.dll imports it.
endlocal
