@echo off
setlocal
cd /d "%~dp0"

set "MSBUILD="
for /f "delimiters=" %%i in ('"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe 2^>nul') do set "MSBUILD=%%i"

if not defined MSBUILD (
    echo MSBuild not found. Open Visual Studio and use: Build -^> Rebuild Solution
    exit /b 1
)

echo Using: %MSBUILD%
"%MSBUILD%" "grk-underwater-world.sln" /p:Configuration=Release /p:Platform=x64 /t:Rebuild
if errorlevel 1 exit /b 1

echo.
echo Build OK. Run:
echo   x64\Release\grk-underwater-world.exe
echo.
pause
