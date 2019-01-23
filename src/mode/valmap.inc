;* VALMAP.INC
;* Copyright (c) 2019 Szieberth Ádám
;* 0BSD License (see LICENSE file for more info)

;* =============================================================================
;* ABSTRACT
;* =============================================================================

;* This file handles the Value Map Screen of the GB-RNG software. This screen
;* shows the values of the tiles as long as the user holds the B button.


;* =============================================================================
;* THE CODE
;* =============================================================================

IF !DEF(VALMAP_INC)
VALMAP_INC SET 1

INCLUDE "MACRO/CP437.INC"
INCLUDE "MACRO/LCD.INC"

INCLUDE "MODE/RANDMAP.INC"


;* As the Value Map Screen has a slave interrupt handler, it should be placed
;* to a fixed address defined in the GBRNG.INC file.
SECTION	"Value Map V-Blank Handler", ROM0[VALMAPVBHADDR]

; The Random Map Screen V-Blank handler checks and handles hold B buttons. If
; the user releases the B button, we go back to Random Map Screen.
valmap_vblankhandler::

;* First we update our key values in the memory, then we check if B button
;* is pressed. If no then we set application mode back to Random Map Screen.
    call key                    ; 3|6+53
    ld a, [KEYNEW]              ; 2|3   A = newly pressed keys
    and a, %00000010            ; 2|2   NZ if B is pressed
    jr nz, .done                 ; 2|3/2

;* If the user released the B button then we set the application mode back to
;* Random Map Screen, unset the slave V-Blank handler, and leave.
.b_released                     ; 8|10
    xor a, a                    ; 1|1   A = 0
    ld [KEYNEW], a              ; 2|3   reset all keys
    ld a, 1                     ; 2|2   A = 1 (next app mode/state: randmap)
    ld [MODE], a                ; 2|3   save next mode
    UnsetVblankHandler          ; 4|5

;* Finally we put the required teardown code to the end of the slave V-Blank
;* handler.
.done                           ; 5|16
    EndOfVblankHandler          ; 5|16


SECTION	"Value Map Screen", ROM0

; This subroutine clears the first column of the screen from row three onwards.
; More precisely it fills that column with emty space (32) tiles.
valmap_col0:
    ld b, 32                    ; 2|2   B = empty space tile
    ld hl, $990C                ; 3|4   start tile address
    jp randmap_fillcol          ; 3|4   clear the column


; This subroutine clears the second column of the screen from row three onwards.
; More precisely it fills that column with emty space (32) tiles.
valmap_col1:
    ld b, 32                    ; 2|2   B = empty space tile
    ld hl, $990D                ; 3|4   start tile address
    jp randmap_fillcol          ; 3|4   clear the column


; This subroutine draws the third column of the Value Map Screen. It draws
; hexadecimal numbers from 0 to F and from top to down.
valmap_col2:
    ld hl, $990E                ; 3|4   start tile address
    ld c, 0                     ; 2|2   hexa value, initially zero
.repeat
    ld a, c                     ; 1|1
    GetHexadecimalTile          ; 8|8/7
    ld b, a                     ; 1|1
    WaitForAnyBlank             ; 6|?
    ld a, b                     ; 1|1   A = vertical line tile
    ld [hl], a                  ; 1|2   write tile to BG map
    ld a, l                     ; 1|1   HL += 32
    add a, 32 % 256             ; 2|2   ...
    ld l, a                     ; 1|1   ...
    ld a, h                     ; 1|1   ...
    adc a, 32 / 256             ; 2|2   ...
    ld h, a                     ; 1|1   ___
    inc c                       ; 1|1   C += 1
    ld a, c                     ; 1|1
    cp a, 16                    ; 2|2
    jr nz, .repeat              ; 2|3/2
    ret                         ; 1|4


; This subroutine draws the value map area of the Value Map Screen which is
; the bottom right 16*16 tiles of the screen. As we are going to display 256
; values, a single byte counter (and offset) will serve us well.
valmap_vals:                    ; 29|?
    ld hl, $9910                ; 3|4   start address
    ld b, 0                     ; 2|2   B = value/counter/offset
.repeat                         ; 24|?
    WaitForAnyBlank             ; 6|?
    ld a, b                     ; 1|1   A = random value
    ld [hl+], a                 ; 1|2   write random value tile to BG map
    inc B                       ; 1|1   B += 1
    ret z                       ; 1|5/2 if B overflowed, we are done
    ld a, b                     ; 1|1   A = value counter/offset
    and a, $0F                  ; 1|1   if the low nibble is nonzero then we are
    jr nz, .repeat              ; 2|3     not at the start of the next row
    ld a, l                     ; 1|1   otherwise, HL += 16
    add a, 16 % 256             ; 2|2   ...
    ld l, a                     ; 1|1   ...
    ld a, h                     ; 1|1   ...
    adc a, 16 / 256             ; 2|2   ...
    ld h, a                     ; 1|1   ___
    jr .repeat                  ; 2|3


; This subroutine draws a new clear Random Map Screen.
valmap_new:                     ; 13|?
    call valmap_col0            ; 3|6+?
    call valmap_col1            ; 3|6+?
    call valmap_col2            ; 3|6+?
    call valmap_vals            ; 3|6+?
    ret                         ; 1|4

ENDC; VALMAP_INC
