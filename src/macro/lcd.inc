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


INCLUDE "HARDWARE.INC"


; Waits for H-Blank or V-Blank whichone comes first. As both are followed by the
; OAM Search period of 20 CPU cycles and 6 is taken by this test, we have 14 CPU
; cycles remaining to write to the VRAM if uninterrupted which should be
; ensured. Thus, STAT interrupt selections should be not set for OAM Search and
; LYC=LC and timer, serial, and joypad interrupts should be also disabled in the
; IE register.
; @destroy A
WaitForAnyBlank: MACRO
.__wait\@
    ld a, [rSTAT]               ; 2|3
    and %00000010               ; 2|2   in mode 2 or 3?
    jr nz, .__wait\@            ; 2|2/3 if no, wait some more
ENDM                            ; 6|? TOTAL


; Waits for V-Blank.
; @destroy A
WaitForVBlank: MACRO
.__wait\@
	ld a, [rLY]                 ; 2|3
	cp a, 144                   ; 2|2   C if row < 144; NC if row >= 144
    jr nc, .__wait\@            ; 2|2/3 if no, wait some more
ENDM                            ; 6|? TOTAL


; Waits for non-V-Blank.
; @destroy A
WaitForNoVBlank: MACRO
.__wait\@
	ld a, [rLY]                 ; 2|3
	cp a, 144                   ; 2|2   C if row < 144; NC if row >= 144
    jr c, .__wait\@             ; 2|2/3 if no, wait some more
ENDM                            ; 6|? TOTAL


; Waits for N frames. N should be set in any non-A register and set the register
; as the first argument. What it does is to wait for N number of no V-Blank
; and N+1 number of V-Blank so that if the given value is zero it waits for the
; first V-Blank state which might be the current LCD mode. Waiting for no
; V-Blanks ensure that the next V-Blank belongs to a different frame.
; @destroy A
WaitFrames: MACRO
    inc \1                      ; 1|1
    jr .__waitforvblank\@       ; 2|3
.__wait\@
    WaitForNoVBlank             ; 6|?
.__waitforvblank\@
    WaitForVBlank               ; 6|?
    dec \1                      ; 1|1
    jr nz, .__wait\@            ; 2|2/3 if no, wait some more
ENDM                            ; 18|? TOTAL

; TODO: WaitFramesWithHalt


ENDC; MACRO_LCD_INC
