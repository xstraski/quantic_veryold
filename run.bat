@echo off
rem ------------------------------
rem Run script for Win32 platform.
rem ------------------------------

rem If no parameters are provided, print usage information.
if [%1]==[] goto PrintUsage
goto RunGame

rem ------------------------------
rem Run the game.
rem ------------------------------
:RunGame
echo Starting %1...
cd build
run%1 -cwd ../base -devmode %2 %3 %4 %6 %7 %8 %9
if %errorlevel%==0 echo %1 exited normally.
if not %errorlevel%==0 echo %1 exited with error(s).
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
