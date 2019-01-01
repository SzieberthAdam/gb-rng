;* PCPYSEED.INC
;* Copyright (c) 2019 Szieberth Ádám
;* 0BSD License (see LICENSE file for more info)

;* =============================================================================
;* ABSTRACT
;* =============================================================================

;* This RGBASM include module provides the ability to squeeze randomness from
;* the initial state of the Game Boy WRAM: "Popcount parity over a large buffer
;* is about as random as you can get." [GBD.PPAR]


;* =============================================================================
;* THE CODE
;* =============================================================================

;* Among other constants, GBRNG.INC contains the address of the random seed
;* which should be used by the RNG if it requires a seed.

INCLUDE "GBRNG.INC"

;* Now we pop the section from the section stack as we will add the 256 bytes
;* long popcount parity lookup table there. Make sure that the start address of
;* table has $00 in the low byte.

POPS

pcpy_data:
    INCBIN "pcpy.bin"           ; 256|0

_PcPy: MACRO
.repeat\@
    ld a, [hl+]                 ; 1|2   loads the next byte
    ld e, a                     ; 1|1   which serves as the table offset
    ld a, [de]                  ; 1|2   get table value (parity of the byte)
    xor a, c                    ; 1|1   adjust parity (add a, c \\ and a, 1)
    ld c, a                     ; 1|1   save parity
.end_check\@
    ld a, h                     ; 1|1   if high byte
    cp a, \1                    ; 2|2   reached the given value
    jr nz, .repeat\@            ; 2|2/3 then done, otherwise continue
.done\@
ENDM                            ; 10|13*N-1 TOTAL


RandomByte: MACRO
    xor a, a                    ; 1|1   clear a
    ld b, a                     ; 1|1   clear b
    ld c, a                     ; 1|1   clear c
REPT 7
    _PcPy \1                    ; 10|13*N-1
    srl c                       ; 2|2   move parity bit to the carry flag
    rl b                        ; 2|2   rotate left b, bit0 = carry flag
    xor a, a                    ; 1|1   clear a
    ld c, a                     ; 1|1   clear c
SHIFT
ENDR
    _PcPy \8                    ; 10|13*N-1
    srl c                       ; 2|2   same as above excluding the final clears
    rl b                        ; 2|2   ___
ENDM                            ; 129|104*N+49


;* The following macro creates four bytes of random seed from the WRAM in
;* a little more time than 0.1 seconds but it requires an extra 533 bytes of
;* space over the popcount parity table's 256 bytes which might be too much for
;* most games.
ExtractWRAMTo32bit: MACRO   ;       N = 256 = $100
    ld d, pcpy_data >> 8        ; 2|2   load constant high byte of table address
    ld hl, $C000                ; 3|3   load WRAM start address
    RandomByte $C1,$C2,$C3,  \  ;       extract one byte
       $C4,$C5,$C6,$C7,$C8      ; 129|26673
    ld a, b                     ; 1|1   and save it in first seed address
    ld [GBRNG_RAMSEED], a       ; 2|2   ___ (LDH)
    RandomByte $C9,$CA,$CB,  \  ;       extract another byte, etc.
       $CC,$CD,$CE,$CF,$D0      ; 129|26673
    ld a, b                     ; 1|1
    ld [GBRNG_RAMSEED+1], a     ; 2|2   LDH
    RandomByte $D1,$D2,$D3,  \
       $D4,$D5,$D6,$D7,$D8      ; 129|26673
    ld a, b                     ; 1|1
    ld [GBRNG_RAMSEED+2], a     ; 2|2   LDH
    RandomByte $D9,$DA,$DB,  \
       $DC,$DD,$DE,$DF,$E0      ; 129|26673
    ld a, b                     ; 1|1
    ld [GBRNG_RAMSEED+3], a     ; 2|2   LDH
ENDM                            ; 533|106709 TOTAL
                                ;       106709/1048576 s ≈ 0.1 s


;* =============================================================================
;* REFERENCES
;* =============================================================================

;* [GBD.PPAR]   ax6 about popcount parity @ gbdev
;*              https://discordapp.com/channels/303217943234215948/303217943234215948/528876463357820928


;* [WP.HW]      Wikipedia: Hamming weight
;*              https://en.wikipedia.org/wiki/Hamming_weight4
