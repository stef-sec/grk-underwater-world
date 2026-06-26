@echo off

setlocal

set ROOT=%~dp0

set MODELS=%ROOT%assets\models



if not exist "%MODELS%" mkdir "%MODELS%"



if not exist "%MODELS%\clownfish.obj" (

    if exist "%USERPROFILE%\Downloads\uploads_files_5014770_Clownfish_Low_Poly.obj" (

        copy /Y "%USERPROFILE%\Downloads\uploads_files_5014770_Clownfish_Low_Poly.obj" "%MODELS%\clownfish.obj" >nul

        echo Copied clownfish.obj from Downloads.

    ) else (

        echo Missing: assets\models\clownfish.obj

    )

) else (

    echo Found: assets\models\clownfish.obj

)



if not exist "%MODELS%\carp.obj" (

    if exist "%USERPROFILE%\Downloads\uploads_files_2372056_carp_with_armature.obj" (

        copy /Y "%USERPROFILE%\Downloads\uploads_files_2372056_carp_with_armature.obj" "%MODELS%\carp.obj" >nul

        echo Copied carp.obj from Downloads.

    ) else (

        echo Missing: assets\models\carp.obj

    )

) else (

    echo Found: assets\models\carp.obj

)



if exist "%MODELS%\seaweed.obj" (

    echo Found: assets\models\seaweed.obj

) else if exist "%MODELS%\seaweedList.obj" (

    echo Found: assets\models\seaweedList.obj

) else (

    echo Optional: copy seaweed.obj to assets\models\

)



exit /b 0

