;* ARITHMET.INC
;* Copyright (c) 2019 Szieberth Ádám
;* 0BSD License (see LICENSE file for more info)

;* =============================================================================
;* ABSTRACT
;* =============================================================================

;* This file contains some RGBDS macros to do various arithmetics with the Game
;* Boy CPU.


; ******************************************************************************
; THE CODE
; ******************************************************************************

IF !DEF(MACRO_ARITHMET_INC)
MACRO_ARITHMET_INC SET 1

; Adds an integer to a 8 bit register (A, B, C, D, E, H, L).
; @return A/B/C/D/E/H/L with the new value.
; @return A with the new value.
Add8: MACRO
    IF STRUPR("\1") != "A"    \
    && STRUPR("\1") != "B"    \
    && STRUPR("\1") != "C"    \
    && STRUPR("\1") != "D"    \
    && STRUPR("\1") != "E"    \
    && STRUPR("\1") != "H"    \
    && STRUPR("\1") != "L"
        FAIL "arg1 should be a register"
    ENDC
    IF STRUPR("\1") == "A"
    add a, \2                   ; 2|2
    ELSE
    ld a, \1                    ; 1|1
    add a, \2                   ; 2|2
    ld \1, a                    ; 1|1
    ENDC
ENDM                            ; 4|4 (2|2 if arg1 is A)


; Adds an integer to a 16 bit register pair (BC, DE, HL).
; @return BC or DE or HL with the new value.
; @return A with the high byte of the new value.
Add16: MACRO
    IF STRUPR("\1") != "HL"   \
    && STRUPR("\1") != "BC"   \
    && STRUPR("\1") != "DE"
        FAIL "arg1 should be a register pair"
    ENDC
_regl EQUS STRSUB("\1", 2, 1)
_regh EQUS STRSUB("\1", 1, 1)
    ld a, _regl                 ; 1|1
    add a, \2 % 256             ; 2|2
    ld _regl, a                 ; 1|1
    ld a, _regh                 ; 1|1
    adc a, \2 / 256             ; 2|2
    ld _regh, a                 ; 1|1
PURGE _regl, _regh
ENDM                            ; 8|8

ENDC ;MACRO_ARITHMET_INC
