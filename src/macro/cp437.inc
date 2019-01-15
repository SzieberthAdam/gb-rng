;* CP437.INC
;* Copyright (c) 2019 Szieberth Ádám
;* 0BSD License (see LICENSE file for more info)

;* =============================================================================
;* ABSTRACT
;* =============================================================================

;* This file contains some RGBDS macros to do various CP437 character map
;* related operations with the Game Boy CPU.


; ******************************************************************************
; THE CODE
; ******************************************************************************

IF !DEF(MACRO_CP437_INC)
MACRO_CP437_INC SET 1


; Converts the value to a tile with the appropriate hexadecimal number.
; @param A: The number whose tile we want. Must be within 0-15 or expect weird
;     results.
; @return A: the tile number
GetHexadecimalTile: MACRO
    cp a, 10                    ; 2|2
    jr c, .inrange09\@          ; 2|3/2
    add a, $7                   ; 2|2
.inrange09\@
    add a, $30                  ; 2|2
ENDM
                                ; 8|8 (8|7 if value in range 0-9)


ENDC; MACRO_CP437_INC
