@echo off
rem -------------------------------------------------------------------------------
rem Build script for Windows-based target platforms.
rem -------------------------------------------------------------------------------
setlocal EnableDelayedExpansion

rem If no parameters are provided, print usage information.
if [%1]==[] goto PrintUsage

rem -----------------------------------
rem General options for compilation
rem and linking.
rem -----------------------------------
rem General project name, must not contain spaces and deprecated symbols, no extension.
rem In a nutshell, the target game platform-specific executable will be named as run%OutputName%.exe,
rem the game engine will be named as %OutputName%.dll,
rem the target game entities will be named as %OutputName%_ents.dll.
set OutputName=quantic

rem -TC					  	   	   	- treat source files only as a pure C code, no C++ at all.
rem -Oi					  			- enable intrinsics generation.
rem -GR-							- disable RTTI.
rem -EHsc							- disable exceptions.
rem -W4								- set warning level to 4.
rem -WX								- treat warnings as errors.
rem -D_CRT_SECURE_NO_WARNINGS=1		- shut VC's screams about some standard CRT functions usage, like snprintf().
rem -DWIN32=1						- signals we are compiling for Windows (some still name it Win32) platform.
set CommonCompilerFlags=-Oi -GR- -EHsc -W4 -WX -wd4201 -wd4127 -D_CRT_SECURE_NO_WARNINGS -DWIN32=1

rem -subsystem:windows				- the program is a pure native Windows application, not a console one.
rem -opt:ref						- remove unreferenced functions and data.
rem -incremental:no					- disable incremental mode.
set CommonLinkerFlags=-subsystem:windows -opt:ref -incremental:no

rem -machine:x86					- target CPU architecture is 32-bit X86.
rem -machine:x64					- target CPU architecture is 64-bit AMD64.
rem -largeaddressaware				- the 32-bit program can handle addresses larger than 2 gigabytes.
if "%1"=="x86" set CPUSpecificLinkerFlags=-machine:x86 -largeaddressaware
if "%1"=="x64" set CPUSpecificLinkerFlags=-machine:x64
if [%1]==[] goto ErrorInvalidParameter

rem -DINTERNAL=1					- [debug] signals we are compiling an internal build, not for public-release.
rem -DINTERNAL=0					- signals we are compiling a public-release build.
rem -MDd							- [debug] use debug multithreaded DLL version of C runtime library.
rem -MD								- use normal multithreaded DLL version of C runtime library.
rem -Zi					  		  	- [debug] include debug info in a program database compatible with Edit&Continue.
rem -Od								- [debug] disable optimization.
if "%2"=="internal:on" set InternalBuildCompilerFlags=-DINTERNAL=1 -MDd -Zi -Od
if "%2"=="internal:off" set InternalBuildCompilerFlags=-DINTERNAL=0 -MD
if [%2]==[] goto ErrorInvalidParameter

rem -DSLOWCODE=1					- [debug] signals we are compiling a paranoid build with slow code enabled.
rem -DSLOWCODE=0					- signals we are compiling a program without any slow code at all.
if "%3"=="slowcode:on" set SlowCodeBuildCompilerFlags=-DSLOWCODE=1
if "%3"=="slowcode:off" set SlowCodeBuildCompilerFlags=-DSLOWCODE=0
if [%3]==[] goto ErrorInvalidParameter

rem -----------------------------------
rem Make build directory.
rem -----------------------------------
if not exist build mkdir build

rem -----------------------------------
rem Clean up previous build.
rem -----------------------------------
call clean.bat

rem -----------------------------------
rem Prepare dependenncies.
rem -----------------------------------
rem Steamworks:
if "%1"=="x86" copy deps\steamworks\redistributable_bin\steam_api.dll build\steam_api.dll > NUL 2> NUL
if "%1"=="x64" copy deps\steamworks\redistributable_bin\win64\steam_api64.dll build\steam_api64.dll > NUL 2> NUL

rem -----------------------------------
rem Build main executable.
rem -----------------------------------
rem Used external libraries, except kernel32.lib:
rem 'user32.lib'  			 		- for windows API general functions.
rem 'gdi32.lib'						- for windows API graphics functions, such as GetDC() or ReleaseDC().
rem 'ole32.lib'						- for Microsoft COM technology.
rem 'comctl32.lib'					- for Microsoft Common Controls.
rem 'winmm.lib'						- for mmsystem.h interface, timeBeginPeriod()/timeEndPeriod().
rem 'opengl32.lib'					- for OpenGL Compatibility-Profile interface.
pushd build
cl -Ferun%OutputName%.exe -Fmrun%OutputName%.map %CommonCompilerFlags% !InternalBuildCompilerFlags! !SlowCodeBuildCompilerFlags! ..\game_platform_win32.cpp /link %CommonLinkerFlags% !CPUSpecificLinkerFlags! user32.lib gdi32.lib ole32.lib comctl32.lib winmm.lib -pdb:run%OutputName%.pdb
set BuildResult=%errorlevel%
popd
if not %BuildResult%==0 goto ErrorBuildFailed

rem -----------------------------------
rem Build game core.
rem -----------------------------------
pushd build
cl -Fe%OutputName%.dll -Fm%OutputName%.map %CommonCompilerFlags% !InternalBuildCompilerFlags! !SlowCodeBuildCompilerFlags! ..\game.cpp /link %CommonLinkerFlags% !CPUSpecificLinkerFlags! -pdb:%OutputName%.pdb -dll -export:GameTrigger
set BuildResult=%errorlevel%
popd
if not %BuildResult%==0 goto ErrorBuildFailed

rem -----------------------------------
rem Build complete.
rem -----------------------------------
goto Eof

rem -----------------------------------
rem Usage information display.
rem -----------------------------------
:PrintUsage
echo BUILD script for Windows target platform.
echo BUILD ^<CPU-type^> ^<internal:on^|off^> ^<slowcode:on^|off^>
echo.
echo CPU-type:
echo * x86            - 32-bit X86-based processors.
echo * x64            - 64-bit X86-based AMD64 processors.
echo.
echo internal:
echo * on             - Internal build.
echo * off            - Public-release build (shipping).
echo.
echo slowcode:
echo * on             - Enable slow code for debugging purpose.
echo * off            - Cut slow code for faster execution.
echo.
goto Eof

rem -----------------------------------
rem Error: invalid parameter.
rem -----------------------------------
:ErrorInvalidParameter
echo ERROR: Invalid parameter detected.
goto PrintUsage

rem -----------------------------------
rem Error: build failed.
rem -----------------------------------
:ErrorBuildFailed
echo ERROR: Build failed.
goto Eof

rem -----------------------------------
rem EOF.
rem -----------------------------------
:Eof
