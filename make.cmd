@echo off
rgbasm -o main.o main.asm
rgblink -p00 -o rngdemo.gb main.o
rgbfix -v -m00 -p00 -t RNGdemo rngdemo.gb
