@echo off
setlocal
set "SRC=C:\Users\Aliaksei\Downloads\uploads_files_2301153_seaweedList.obj"
set "DST=%~dp0assets\models\seaweedList.obj"
if not exist "%~dp0assets\models" mkdir "%~dp0assets\models"
if not exist "%SRC%" (
    echo Source not found: %SRC%
    exit /b 1
)
copy /Y "%SRC%" "%DST%"
echo Copied seaweed model to %DST%
