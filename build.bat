@echo off

copy Screen.jack ScreenTest
REM copy Math.jack ScreenTest
call ..\..\tools\JackCompiler.bat ScreenTest
