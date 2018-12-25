@echo off
rgbasm -o main.o main.asm
rgblink -p00 -o gbrng.gb main.o
rgbfix -v gbrng.gb
