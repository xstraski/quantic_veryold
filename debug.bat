@echo off
rem ------------------------------
rem Debug script for Win32 platform.
rem ------------------------------

rem If no parameters are provided, print usage information.
if [%1]==[] goto PrintUsage
goto RunDebug

rem ------------------------------
rem Run the debugger.
rem ------------------------------
:RunDebug
cd build
devenv run%1.exe
cd ..
goto Eof

rem ------------------------------
rem Usage.
rem ------------------------------
:PrintUsage
echo DEBUG script for Windows target platform.
echo DEBUG ^<shared-name^>
echo.
goto Eof

rem ------------------------------
rem Eof.
rem ------------------------------
:Eof
