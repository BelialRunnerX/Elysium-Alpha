@echo off
REM Configure the project and open the generated solution in Visual Studio.
REM
REM Visual Studio 2022 can also just open this folder directly -- File > Open >
REM Folder -- because CMakePresets.json is here and VS reads it natively. This
REM script is for when you would rather have a .sln.

setlocal

if "%VCPKG_ROOT%"=="" (
  echo.
  echo   VCPKG_ROOT is not set, so zlib, GLFW and EnTT cannot be found.
  echo.
  echo   Install vcpkg once:
  echo     git clone https://github.com/microsoft/vcpkg C:\vcpkg
  echo     C:\vcpkg\bootstrap-vcpkg.bat
  echo     setx VCPKG_ROOT C:\vcpkg
  echo.
  echo   Then open a NEW terminal and run this again.
  echo.
  pause
  exit /b 1
)

echo Configuring... the first run takes a few minutes while vcpkg builds
echo zlib, GLFW and EnTT. After that it is seconds.
echo.
cmake --preset windows
if errorlevel 1 (
  echo.
  echo   Configure failed. The message above says why.
  pause
  exit /b 1
)

echo.
echo Opening build\elysium.sln
start "" "build\elysium.sln"
endlocal
