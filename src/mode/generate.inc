;* GENERATE.INC
;* Copyright (c) 2019 Szieberth Ádám
;* 0BSD License (see LICENSE file for more info)

;* =============================================================================
;* ABSTRACT
;* =============================================================================

;* This file handles the Random Number Generating Screen of the GB-RNG software.


; ******************************************************************************
; THE CODE
; ******************************************************************************

IF !DEF(GENERATE_INC)
GENERATE_INC SET 1

INCLUDE "MACRO/ARITHMET.INC"
INCLUDE "MACRO/CP437.INC"
INCLUDE "MACRO/LCD.INC"


SECTION	"Random Number Generating Screen", ROM0


; This subroutine draws a new clear Random Number Generating Screen. The tile
; values are stored at the `generate_background` label in 96 bytes. Zero values
; represent tiles to be skipped.
generate_new:
    ld hl, generate_background  ; 3|4   tile data
    ld de, $9990                ; 3|4   destination start address (VRAM)
.repeat
    ld a, [hl+]                 ; 1|2   A = tile
    and a, a                    ; 1|1   ≡ cp a, 0
    jr z, .afterwrite           ; 2|3/2 if zero, go to next without displaying
    ld b, a                     ; 1|1   B = tile
    WaitForBlanking             ; 6|?
    ld a, b                     ; 1|1   A = tile
    ld [de], a                  ; 1|2   write tile to BG map
.afterwrite
    inc de                      ; 1|1   increment destination address
    ld a, l                     ; 1|1   after 96 tiles, we are done
    and a, %01111111            ; 2|2   ...
    cp a, 96                    ; 2|2   ...
    ret z                       ; 1|5/2 ___
    and a, $0F                  ; 2|2   if not at row end (every 16), go to next
    jr nz, .repeat              ; 2|3/2 ___
    Add16 de, 16                ; 8|8   at row end, adjust DE; DE += 16
    jr .repeat                  ; 2|3


SECTION	"Random Number Generating Screen Background", ROM0, ALIGN[7]
generate_background:
    INCBIN "MODE/GENERATE.BIN"


ENDC; GENERATE_INC
