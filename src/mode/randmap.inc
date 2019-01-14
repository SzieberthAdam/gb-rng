;* RANDMAP.INC
;* Copyright (c) 2019 Szieberth Ádám
;* 0BSD License (see LICENSE file for more info)

;* =============================================================================
;* ABSTRACT
;* =============================================================================

;* This file handles the Random Map Screen of the GB-RNG software.


; ******************************************************************************
; THE CODE
; ******************************************************************************

IF !DEF(RANDMAP_INC)
RANDMAP_INC SET 1

INCLUDE "MACRO/ARITHMET.INC"
INCLUDE "MACRO/CP437.INC"
INCLUDE "MACRO/LOAD.INC"
INCLUDE "MACRO/LCD.INC"


SECTION	"Random Map Screen", ROM0


; This subroutine initializes the Random Map Screen.
randmap_init:
    Load16 [DISPOFFSET], $C000  ; 7|9   reset the view offset
    ret                         ; 1|4


; This subroutine draws the first row of the Random Map Screen. The first four
; tiles there should be empty spaces (32) and the remaining tiles to be
; hexadecimal numbers from 0 to F.
randmap_row0:
    ld hl, $98CC                ; 3|4   start tile address
    ld b, 32                    ; 2|2   B = space tile
.repeat
    WaitForBlanking             ; 6|?
    ld a, b                     ; 1|1   A = tile
    ld [hl+], a                 ; 1|2   write tile to BG map
    ld a, l                     ; 1|1   at $98E0 we are done
    cp a, $E0                   ; 2|2   ...
    ret z                       ; 1|5/2 ___
    ld a, l                     ; 1|1
    and a, %00011111            ; 2|2   A = column
    sub a, $D0 & %00011111      ; 2|2   A = 0 should be at $98D0
    jr c, .repeat               ; 2|3/2 if A < 0 -> B remains space tile
    GetHexadecimalTile          ; 8|8/7
    ld b, a                     ; 1|1   B = hexadecimal tile
    jr .repeat                  ; 2|3


; This subroutine draws the second row of the Random Map Screen. The first
; three tiles there should be empty spaces (32), the fourth tile should be an
; CP437 single line topleft corner (218) and the remaining tiles should be
; horizontal box drawing lines (196).
randmap_row1:
    ld hl, $98EC                ; 3|4   start tile address
    ld b, 32                    ; 2|2   B = space tile
.repeat
    WaitForBlanking             ; 6|?
    ld a, b                     ; 1|1   A = tile
    ld [hl+], a                 ; 1|2   write tile to BG map
    ld a, l                     ; 1|1   at $9900 we are done
    and a, a                    ; 1|1   ...; ≡ cp a, 0
    ret z                       ; 1|5/2 ___
    ld a, l                     ; 1|1
    sub a, $EF                  ; 2|2   are we at $98EF?
    jr c, .repeat               ; 2|3/2 if before -> B remains space tile
    jr z, .tile218              ; 2|3/2 if yes -> corner tile
    ld b, 196                   ; 2|2   if after -> B = hline tile
    jr .repeat                  ; 2|3
.tile218
    ld b, 218                   ; 2|2   B = corner tile
    jr .repeat                  ; 2|3


; This subroutine draws the first column of the row addresses of the Random Map
; Screen. These are the high nibbles of the high bytes of the addresses of the
; random values in the appropriate row of the random map. That address increases
; by 16 with every row: address = [DISPOFFSET] + 16 * randmaprow. We render the
; tiles going downwards vertically.
randmap_addrcol0:
    ld hl, $990C                ; 3|4   start tile address
    ld c, 0                     ; 2|2   C = 16 * randmaprow
.repeat
    ld a, [DISPOFFSET+1]        ; 2|3   A = topleft address low byte
    add a, c                    ; 1|1   A = current address low byte
    ld a, [DISPOFFSET]          ; 2|3   A = topleft address high byte
    adc a, 0                    ; 2|2   A = current address high byte (AHB)
    and a, $F0                  ; 2|2   trim the low nibble
    swap a                      ; 2|2   A = AHB high nibble
    GetHexadecimalTile          ; 8|7/8
    ld b, a                     ; 1|1   B = hexadecimal tile
    WaitForBlanking             ; 6|?
    ld a, b                     ; 1|1   A = hexadecimal tile
    ld [hl], a                  ; 1|2   write tile to BG map
    Add16 hl, 32                ; 8|8   HL += 32
    cp a, $9B                   ; 2|2   at $9B0C we are done
    ret z                       ; 1|5/2 ___
    Add8 c, 16                  ; 4|4   C += 16
    jr .repeat                  ; 2|3


; This subroutine draws the second column of the row addresses of the Random Map
; Screen. These are the low nibbles of the high bytes of the addresses of the
; random values in the appropriate row of the random map. That address increases
; by 16 with every row: address = [DISPOFFSET] + 16 * randmaprow. We render the
; tiles going downwards vertically.
randmap_addrcol1:
    ld hl, $990D                ; 3|4   start tile address
    ld c, 0                     ; 2|2   C = 16 * randmaprow
.repeat
    ld a, [DISPOFFSET+1]        ; 2|3   A = topleft address low byte
    add a, c                    ; 1|1   A = current address low byte
    ld a, [DISPOFFSET]          ; 2|3   A = topleft address high byte
    adc a, 0                    ; 2|2   A = current address high byte (AHB)
    and a, $0F                  ; 2|2   A = AHB low nibble
    GetHexadecimalTile          ; 8|7/8
    ld b, a                     ; 1|1   B = hexadecimal tile
    WaitForBlanking             ; 6|?
    ld a, b                     ; 1|1   A = hexadecimal tile
    ld [hl], a                  ; 1|2   write tile to BG map
    Add16 hl, 32                ; 8|8   HL += 32
    cp a, $9B                   ; 2|2   at $9B0D we are done
    ret z                       ; 1|5/2 ___
    Add8 c, 16                  ; 4|4   C += 16
    jr .repeat                  ; 2|3


; This subroutine draws the third column of the row addresses of the Random Map
; Screen. These are the high nibbles of the low bytes of the addresses of the
; random values in the appropriate row of the random map. That address increases
; by 16 with every row: address = [DISPOFFSET] + 16 * randmaprow. We render the
; tiles going downwards vertically.
randmap_addrcol2:
    ld hl, $990E                ; 3|4   start tile address
    ld c, 0                     ; 2|2   C = 16 * randmaprow
.repeat
    ld a, [DISPOFFSET+1]        ; 2|3   A = topleft address low byte
    add a, c                    ; 1|1   A = current address low byte (ALB)
    and a, $F0                  ; 2|2   trim the low nibble
    swap a                      ; 2|2   A = ALB high nibble
    GetHexadecimalTile          ; 8|7/8
    ld b, a                     ; 1|1
    WaitForBlanking             ; 6|?
    ld a, b                     ; 1|1   A = hexadecimal tile
    ld [hl], a                  ; 1|2   write tile to BG map
    Add16 hl, 32                ; 8|8   HL += 32
    cp a, $9B                   ; 2|2   at $9B0E we are done
    ret z                       ; 1|5/2 ___
    Add8 c, 16                  ; 4|4   C += 16
    jr .repeat                  ; 2|3


; This subroutine draws the fourth column of the row addresses of the Random Map
; Screen. These are CP437 single vertical box drawing lines (179).
randmap_addrcol3:
    ld hl, $990F                ; 3|4   start tile address
    ld b, 179                   ; 2|2   B = vertical line tile
    ld c, 0                     ; 2|2   C = 16 * randmaprow
.repeat
    WaitForBlanking             ; 6|?
    ld a, b                     ; 1|1   A = vertical line tile
    ld [hl], a                  ; 1|2   write tile to BG map
    Add16 hl, 32                ; 8|8   HL += 32
    cp a, $9B                   ; 2|2   at $9B0F we are done
    ret z                       ; 1|5/2 ___
    Add8 c, 16                  ; 4|4   C += 16
    jr .repeat                  ; 2|3


; This subroutine clears the random map area of the Random Map Screen which is
; the bottom right 16*16 tiles of the screen. As we are going to display 256
; empty space (32) values, a single byte counter (and offset) will serve us
; well.
clear_randmap_vals:
    ld hl, $9910                ; 3|4   start address
    ld b, 32                    ; 2|2   B = space tile
    ld c, 0                     ; 2|2   C = value counter/offset
.repeat
    WaitForBlanking             ; 6|?
    ld a, b                     ; 1|1   A = space tile
    ld [hl+], a                 ; 1|2   write space tile to BG map
    inc c                       ; 1|1   C += 1
    ret z                       ; 1|5/2 if C overflowed, we are done
    ld a, c                     ; 1|1   A = value counter/offset
    and a, $0F                  ; 1|1   if the low nibble is nonzero then we are
    jr nz, .repeat              ; 2|3     not at the start of the next row
    Add16 HL, 16                ; 8|8   otherwise, HL += 16
    jr .repeat                  ; 2|3


; This subroutine draws the random map area of the Random Map Screen which is
; the bottom right 16*16 tiles of the screen. As we are going to display 256
; values, a single byte counter (and offset) will serve us well.
randmap_vals:
    ld hl, $9910                ; 3|4   start address
    ld c, 0                     ; 2|2   C = value counter/offset
.repeat
    ld a, [DISPOFFSET+1]        ; 2|3   A = topleft address low byte
    add a, c                    ; 1|1   A = current address low byte
    ld e, a                     ; 1|1   E = current address low byte
    ld a, [DISPOFFSET]          ; 2|3   A = topleft address high byte
    adc a, 0                    ; 2|2   A = current address high byte
    ld d, a                     ; 1|1   D = current address high byte
    ld a, [de]                  ; 1|2   A = random value
    ld b, a                     ; 1|1   B = random value
    WaitForBlanking             ; 6|?
    ld a, b                     ; 1|1   A = random value
    ld [hl+], a                 ; 1|2   write random value tile to BG map
    inc c                       ; 1|1   C += 1
    ret z                       ; 1|5/2 if C overflowed, we are done
    ld a, c                     ; 1|1   A = value counter/offset
    and a, $0F                  ; 1|1   if the low nibble is nonzero then we are
    jr nz, .repeat              ; 2|3     not at the start of the next row
    Add16 HL, 16                ; 8|8   otherwise, HL += 16
    jr .repeat                  ; 2|3


; This subroutine draws a new clear Random Map Screen.
randmap_new:
    call randmap_init
    call randmap_row0
    call randmap_row1
    call randmap_addrcol0
    call randmap_addrcol1
    call randmap_addrcol2
    call randmap_addrcol3
    call clear_randmap_vals
    ret                         ; 1|4

ENDC; RANDMAP_INC
