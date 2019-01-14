;* MO_LOAD.INC
;* Copyright (c) 2019 Szieberth Ádám
;* 0BSD License (see LICENSE file for more info)

;* =============================================================================
;* ABSTRACT
;* =============================================================================

;* This file contains some RGBDS macros to do various load operations with the
;* Game Boy CPU.


; ******************************************************************************
; THE CODE
; ******************************************************************************

IF !DEF(MO_LOAD_INC)
MO_LOAD_INC SET 1

; Dumps a given constant number into two bytes of the RAM. The high byte goes to
; the given address and the low byte to the given address + 1.
; @destroy A
Load16: MACRO
_len = STRLEN("\1")
_fi = STRSUB("\1", 1, 1)
_la = STRSUB("\1", _len, 1)
    IF _fi != "[" && _la != "]"
        FAIL "arg1 should be an a16 in square brackets"
    ENDC
_a16 EQUS STRSUB("\1", 2, _len + (-2))
    IF (\2 % 256) == 0
        xor a, a                ; 1|1
    ELSE
        ld a, (\2 % 256)        ; 2|2
    ENDC
    ld [_a16+1], a              ; 2/3|3/4
    IF (\2 % 65536) >> 8 == 0
        xor a, a                ; 1|1
    ELSE
        ld a, (\2 % 65536) >> 8 ; 2|2
    ENDC
    ld [_a16], a                ; 2/3|3/4
PURGE _len, _fi, _la, _a16
ENDM


ENDC ;MO_LOAD_INC
