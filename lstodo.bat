@echo off
rem -------------------------------------------------------------------------------
rem TODOs listing script for Win32 platform.
rem -------------------------------------------------------------------------------

set Wildcard=*.cpp *h
findstr "TODO(" %Wildcard%
