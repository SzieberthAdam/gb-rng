@echo off

if "%~1"=="" goto usage

if not exist %1.asm goto nofile

:create_rom
rgbasm -o main.o main.asm
rgbasm -o %1.o %1.asm
rgblink -d -t -p00 -o gb%1.gb main.o %1.o
rgbfix -v gb%1.gb
goto end

:nofile
echo File not found: %1.asm
goto end

:usage
echo USAGE: make RNG-NAME
echo A file named RNG-NAME.ASM should exists in the root directory.
echo The output file will be named as GBRNG-NAME.GB.

:end
