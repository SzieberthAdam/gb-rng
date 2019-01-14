@echo off

if "%~1"=="" goto noarg

if not exist %1.asm goto nofile

:create_rom
rgbasm -v -o gbrng.o gbrng.asm
rgbasm -v -o %1.o %1.asm
rgblink -d -t -p00 -o gb%1.gb %1.o gbrng.o
rgbfix -v gb%1.gb
goto end

:nofile
echo File not found: %1.asm
goto end

:noarg
rgbasm -v -o gbrng.o gbrng.asm
for /f "usebackq delims=|" %%f in (`dir /b "%~dp0"\\rng-*.asm`) do (
    rgbasm -v -o %%~nf.o %%~nf.asm
    rgblink -d -t -p00 -o gb%%~nf.gb %%~nf.o gbrng.o
    rgbfix -v gb%%~nf.gb
)

:end
