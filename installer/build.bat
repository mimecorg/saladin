@echo off

if "%1" == "" goto usage

set VERSION=0.5
set BUILDVERSION=0.5.0.5241

set NSISDIR=C:\Program Files (x86)\NSIS\Unicode

if not %1 == x86 goto elsex64

set BUILDDIR=D:\Release\saladin-x86

set QTDIR=D:\Qt\4.8-x86-static

set ARCHITECTURE=win_x86
set SUFFIX=

goto endopt
:elsex64

if not %1 == x64 goto usage

set BUILDDIR=D:\Release\saladin-x64

set QTDIR=D:\Qt\4.8-x64-static

set ARCHITECTURE=win_x64
set SUFFIX= (64-bit)

goto endopt
:usage

echo usage: build-bat x86^|x64

goto :eof
:endopt

call "%QTDIR%\bin\qtvars.bat" vsvars

if not exist "%BUILDDIR%" mkdir "%BUILDDIR%"

echo.
echo Exporting...
echo.

svn export --quiet --force .. "%BUILDDIR%"

pushd "%BUILDDIR%"

call configure.bat

nmake release

if errorlevel 1 goto cleanup

call ..\sign.bat release\saladin.exe

echo.
echo Creating uninstaller...
echo.

"%NSISDIR%\makensis.exe" /DINNER "/DQTDIR=%QTDIR%" "/DVERSION=%VERSION%" "/DBUILDVERSION=%BUILDVERSION%" "/DARCHITECTURE=%ARCHITECTURE%" "/DSUFFIX=%SUFFIX%" /V2 installer\saladin.nsi

if errorlevel 1 goto cleanup

"%TEMP%\innerinst.exe"

call ..\sign.bat "%TEMP%\uninstall.exe"

echo.
echo Creating installer...
echo.

"%NSISDIR%\makensis.exe" "/DQTDIR=%QTDIR%" "/DVERSION=%VERSION%" "/DBUILDVERSION=%BUILDVERSION%" "/DARCHITECTURE=%ARCHITECTURE%" "/DSUFFIX=%SUFFIX%" /V2 installer\saladin.nsi

if errorlevel 1 goto cleanup

call ..\sign.bat "installer\saladin-%VERSION%-%ARCHITECTURE%.exe"

:cleanup

del /q /f "%TEMP%\innerinst.exe"
del /q /f "%TEMP%\uninstall.exe"

popd
