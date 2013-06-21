@echo off
echo.

setlocal
chdir /d "%~dp0"

rem set WDKBASEDIR=C:\WinDDK\7600.16385.1
set WDKBASEDIR=D:\Programme\Microsoft wdk7

if /i "%1" == "i386" call :i386
if /i "%1" == "amd64" call :amd64
if "%CFG%" == "" call :notarget && exit /b
if /i "%2" == "clean" set CMD=clean
if /i "%2" == "rebuild" set CMD=clean build

nmake /nologo /f Notepad2.mak CFG=%CFG% %CMD%

:i386
set CFG=i386
set CMD=build
set PATH=%WDKBASEDIR%\bin\x86;%WDKBASEDIR%\bin\x86\x86;%PATH%
set INCLUDE=%WDKBASEDIR%\inc\crt;%WDKBASEDIR%\inc\api;%WDKBASEDIR%\inc\api\crt\stl60;%WDKBASEDIR%\inc\ddk
set LIB=%WDKBASEDIR%\lib\crt\i386;%WDKBASEDIR%\lib\win7\i386
exit /b

:amd64
set CFG=amd64
set CMD=build
set PATH=%WDKBASEDIR%\bin\x86;%WDKBASEDIR%\bin\x86\amd64;%PATH%
set INCLUDE=%WDKBASEDIR%\inc\crt;%WDKBASEDIR%\inc\api;%WDKBASEDIR%\inc\api\crt\stl60;%WDKBASEDIR%\inc\ddk
set LIB=%WDKBASEDIR%\lib\crt\amd64;%WDKBASEDIR%\lib\win7\amd64
exit /b

:notarget
echo Specify target: i386 or amd64.
exit /b
