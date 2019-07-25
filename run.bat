@echo off
rem ------------------------------
rem Run script for Win32 platform.
rem ------------------------------

rem General project name, must not contain spaces and deprecated symbols, no extension.
rem In a nutshell, the target game platform-specific executable will be named as run%OutputName%.exe,
rem the game engine will be named as %OutputName%.dll,
rem the target game entities will be named as %OutputName%_ents.dll.
set OutputName=%1

rem If no parameters are provided, print usage information.
if [%OutputName%]==[] goto PrintUsage
goto RunGame

rem ------------------------------
rem Run the game.
rem ------------------------------
:RunGame
echo Starting %OutputName%...
cd build
run%OutputName% -cwd ../b%OutputName% -devmode %2 %3 %4 %6 %7 %8 %9
if %errorlevel%==0 echo %OutputName% exited normally.
if not %errorlevel%==0 echo %OutputName% exited with error(s).
cd ..
goto Eof

rem ------------------------------
rem Usage.
rem ------------------------------
:PrintUsage
echo RUN script for Windows target platform.
echo RUN ^<shared-name^>
echo.
goto Eof

rem ------------------------------
rem Eof.
rem ------------------------------
:Eof
