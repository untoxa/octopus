@echo off
set BIN=..\gbdk-n\bin
set OBJ=build

if "%1"=="clean" (
	if exist %OBJ% rd /s/q %OBJ%
	if exist octopus.gb del octopus.gb
	goto end
)

if not exist %OBJ% mkdir %OBJ%
call %BIN%\gbdk-n-compile.bat src\octopus.c -o %OBJ%\octopus.rel
call %BIN%\gbdk-n-link.bat %OBJ%\octopus.rel -o %OBJ%\octopus.ihx
call %BIN%\gbdk-n-make-rom.bat %OBJ%\octopus.ihx octopus.gb

:end
