@echo off
rem -------------------------------------------------------------------------------
rem Clean up script for Windows-based target platforms.
rem -------------------------------------------------------------------------------
pushd build

rmdir /S /Q .vs > NUL 2> NUL
del /S *.obj > NUL 2> NUL
del /S *.dll > NUL 2> NUL
del /S *.exp > NUL 2> NUL
del /S *.lib > NUL 2> NUL
del /S *.map > NUL 2> NUL
del /S *.pdb > NUL 2> NUL
del /S *.exe > NUL 2> NUL
del /S *.manifest > NUL 2> NUL

popd
