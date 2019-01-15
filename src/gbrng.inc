IF !DEF(GBRNG_INC)
GBRNG_INC = 1

GENERATEVBHADDR EQU $0068

RNGREGST EQU $FF90              ;       8 bytes for AF BC DE HL

SLAVEVBHADDR EQU $FFA1
KEYNEW EQU $FFA3
KEYOLD EQU $FFA4

GBRNG_DSP_ROW EQU $FFAA
DISPOFFSET EQU $FFAC
GENERATEADDR EQU $FFAE


DISPTSCX EQU 12
DISPTSCY EQU 6


; This macro unsets the mode specific V-Blank handler. A 16bit address should be
; given as an argument. As the master V-Blank handler passes the handling when
; the high byte is not $E0, we write the low byte first as we do not want to be
; redirected to a partly set memory address.
; @destroy A
SetVblankHandler: MACRO
    ld a, \1 & $FF              ; 2|2   A = low byte
    ld [SLAVEVBHADDR+1], a      ; 2|3
    ld a, \1 >> 8               ; 2|2   A = high byte
    ld [SLAVEVBHADDR], a        ; 2|3
ENDM                            ; 8|10 TOTAL


; This macro unsets the mode specific V-Blank handler.
; @destroy A
UnsetVblankHandler: MACRO
    ld a, $E0                   ; 2|2
    ld [SLAVEVBHADDR], a        ; 2|3
ENDM                            ; 4|5 TOTAL


; IMPORTANT!
; This macro must be called at the end of every slave V-Blank handler!
; It restores the registers and returns from the interrupt.
EndOfVblankHandler: MACRO
    pop hl                      ; 1|3
    pop de                      ; 1|3
    pop bc                      ; 1|3
    pop af                      ; 1|3
    reti                        ; 1|4
ENDM                            ; 5|16 TOTAL


ENDC ;GBRNG_INC
