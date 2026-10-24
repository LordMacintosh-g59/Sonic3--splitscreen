@echo off
chcp 65001 > nul
title Sonic 3 ^& Knuckles - Co-op

:: A intro (Misc\lordmacintosh_abertura.mp4) toca dentro do jogo, a partir de
:: Oxygen\sonic3air\data\images\intro e data\audio\original\coop_intro.ogg
set "GAME_DIR=%~dp0Oxygen\sonic3air\bin\Release_x64"

if exist "%GAME_DIR%\Sonic3AIR.exe" (
    start "" /d "%GAME_DIR%" "%GAME_DIR%\Sonic3AIR.exe"
    goto :EOF
)

echo.
echo  Sonic3AIR.exe nao encontrado em:
echo    %GAME_DIR%
echo  Compile o projeto (Oxygen\sonic3air\build\_vstudio\sonic3air.sln, Release x64).
echo.
pause
