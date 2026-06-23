@echo off
setlocal
echo Place your seaweed OBJ in: %~dp0assets\models\seaweed.obj
if exist "%~dp0assets\models\seaweed.obj" (
    echo Found: assets\models\seaweed.obj
    exit /b 0
)
if exist "%~dp0assets\models\seaweedList.obj" (
    echo Found: assets\models\seaweedList.obj
    exit /b 0
)
echo Missing model. Copy seaweed.obj to assets\models\
exit /b 1
