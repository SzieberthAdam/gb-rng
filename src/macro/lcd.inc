;* LCD.INC
;* Copyright (c) 2019 Szieberth Ádám
;* 0BSD License (see LICENSE file for more info)

;* =============================================================================
;* ABSTRACT
;* =============================================================================

;* This file contains some RGBDS macros to do various operations with the Game
;* Boy LCD.


; ******************************************************************************
; THE CODE
; ******************************************************************************

IF !DEF(MACRO_LCD_INC)
MACRO_LCD_INC SET 1


; Waits for H-Blank or V-Blank whichone comes first. As both are followed by the
; OAM Search period of 20 CPU cycles and 6 is taken by this test, we have 14 CPU
; cycles remaining to write to the VRAM if uninterrupted which should be
; ensured. Thus, STAT interrupt selections should be not set for OAM Search and
; LYC=LC and timer, serial, and joypad interrupts should be also disabled in the
; IE register.
; @destroy A
WaitForBlanking: MACRO
.__wait@
    ld a, [rSTAT]               ; 2|3
    and %00000010               ; 2|2   in mode 2 or 3?
    jr nz, .__wait@             ; 2|2/3 if no, wait some more
ENDM


ENDC; MACRO_LCD_INC
