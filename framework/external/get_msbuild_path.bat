@echo off

:: You might need to change this path to point to your Visual Studio installation's MSBuild.exe
set msbuildPathCom="C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
set msbuildPathPro="C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
set msbuildPathBT="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"

if exist %msbuildPathPro% (
	set msbuildPath=%msbuildPathPro%
) else if exist %msbuildPathCom% (
	set msbuildPath=%msbuildPathCom%
) else (
	set msbuildPath=%msbuildPathBT%
)

if not exist %msbuildPath% (
	echo Error: No Visual Studio 2022 installation found
	pause
	exit /b 1
)

echo Using Visual Studio installation: %msbuildPath%
echo.
