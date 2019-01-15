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

INCLUDE "GBRNG.INC"
INCLUDE "MACRO/ARITHMET.INC"
INCLUDE "MACRO/CP437.INC"
INCLUDE "MACRO/LCD.INC"
INCLUDE "MACRO/REGISTER.INC"

;* As the Generating Screen has a slave interrupt handler, it should be placed
;* to a fixed address defined in the GBRNG.INC file.
SECTION	"Generate V-Blank Handler", ROM0[GENERATEVBHADDR]

; The Generating Screen V-Blank handler does three things in decreasing
; priority: (1) updates the progress bar, (2) updates the address of the random
; number which is being calculated, and (3) renders the generated random values
; around the pop-up window onto the random map area.
generate_vblankhandler::

;* Our progress bar is 8 tiles wide and our addresses are in range $C000--$E000.
;* Thus, after every $400 a progress bar tile should be set. To make the
;* progress bar biased, we will set tiles at the following address high bytes:
;* $C2, $C6, $CA, $CE, $D2, $D6, $DA, $DE. Lets examine those binary values!
;*
;* $C0 11000000 nothing to do
;* $C1 11000001 nothing to do
;* $C2 11000010 1st progress bar tile; bit 1 indicates that; bit 2--4 index = 0
;* $C6 11000110 2nd; bit 1 set; bit 2--4 index = 1
;* $CA 11001010 3rd; bit 1 set; bit 2--4 index = 2
;* $CE 11001110 ...
;* $D2 11010010 ...
;* $D6 11010110 ...
;* $DA 11011010 ...
;* $DE 11011110 8th; bit 1 set; bit 2--4 index = 7
;* $DF 11011111 keep the 8th; bit 1 set; bit 2--4 index = 7
;* $E0 11100000 keep the 8th; bit 1 reset <-> nothing to do
;*
;* This means that we skip the progress bar update if bit 1 is reset and we get
;* our horizontal offset index from bits 2--4.
.update_progressbar
    LoadA GENERATEADDR          ; 2|3   A = high byte of the randval address
    ld b, a                     ; 1|1   B = high byte of the randval address
    and a, %00000010            ; 2|2   test for bit 1
    jr z, .update_randval_addr ; 2|3/2
    ld a, b                     ; 1|1   A = high byte of the randval address
    and a, %00011100            ; 2|2   keep bit 2--4, offset index bits
    rrca                        ; 1|1   rotate right
    rrca                        ; 1|1   rotate right; A = progress bar offset
    ld hl, $99F2                ; 3|4   first progress bar tile BG map address
    add a, l                    ; 1|1   add the progress bar offset to L
    ld l, a                     ; 1|1   ___
    ld a, $DB                   ; 2|2   A = progress bar set tile
    ld [hl], a                  ; 1|2   write tile to the BG map
.update_randval_addr
    ld hl, $9A14                ; 3|4   first randval addr. tile BG map address
    ld a, b                     ; 1|1   A = high byte of the randval addr. (AHB)
    and a, $F0                  ; 2|2   trim the low nibble
    swap a                      ; 2|2   A = AHB high nibble
    GetHexadecimalTile          ; 8|7/8
    ld [hl+], a                 ; 1|2   write tile to BG map; HL += 1
    ld a, b                     ; 1|1   A = AHB
    and a, $0F                  ; 2|2   A = AHB low nibble
    GetHexadecimalTile          ; 8|8/7
    ld [hl+], a                 ; 1|2   write tile to BG map; HL += 1
    LoadA GENERATEADDR+1        ; 2|3   A = low byte of the randval addr. (ALB)
    ld b, a                     ; 1|1   B = ALB
    and a, $F0                  ; 2|2   trim the low nibble
    swap a                      ; 2|2   A = ALB high nibble
    GetHexadecimalTile          ; 8|7/8
    ld [hl+], a                 ; 1|2   write tile to BG map; HL += 1
    ld a, b                     ; 1|1   A = ALB
    and a, $0F                  ; 2|2   A = ALB low nibble
    GetHexadecimalTile          ; 8|8/7
    ld [hl], a                  ; 1|2   write tile to BG map
.update_random_map              ;       TODO!!!
.done
    EndOfVblankHandler          ; 5|16


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
    WaitForAnyBlank             ; 6|?
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


; This subroutine initialize/reset the attached RNG and dumps all of the
; registers into the HRAM thereafter so that these registers can be set back
; every time the RNG is called to generate a new random number.
reset_rng:
    call rand_init              ; 3|6
    DumpAllRegisters RNGREGST   ; 28|35
    ret                         ; 1|4

generate_rand_data:
    SetVblankHandler $0068      ; 8|10  set generate_vblankhandler as slave
    ld hl, $C000                ; 3|4   set HL to destination address
    DumpHL GENERATEADDR         ; 6|6
.repeat
    LoadAllRegisters RNGREGST   ; 27|40
    call rand                   ; 3|6   random byte -> [RNGREGST]
    DumpAllRegisters RNGREGST   ; 28|43
    LoadHL GENERATEADDR         ; 6|8
    LoadA RNGREGST              ; 2|3
    ld [hl+], a                 ; 1|2   loads value to destination address
    DumpHL GENERATEADDR         ; 6|8   this also increments the fake rDIV
.end_check                      ;       HL == $9A34; .done; .end_check_break
    ld a, h                     ; 1|1
    cp $E0                      ; 2|2
    jr nz, .repeat              ; 2|2/3
    ld a, l                     ; 1|1
    and a, a                    ; 1|1   ≡ cp a, 0
    jr nz, .repeat              ; 2|2/3
.done
    ld a, 1                     ; 2|2   wait one frame to show the final state
    ld c, a                     ; 1|1   ...
    WaitFrames c                ; 18|?  ___
    UnsetVblankHandler          ; 4|5
    ret                         ; 1|4


SECTION	"Random Number Generating Screen Background", ROM0, ALIGN[7]
generate_background:
    INCBIN "MODE/GENERATE.BIN"


ENDC; GENERATE_INC
