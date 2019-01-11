;* RNG-ION
;* Copyright (c) 2019 Szieberth Ádám
;* 0BSD License (see LICENSE file for more info)


;* =============================================================================
;* ABSTRACT
;* =============================================================================

;* This is based off the tried and true pseudorandom number generator featured
;* in Ion by Joe Wingbermuehle.


;* =============================================================================
;* CHANGES
;* =============================================================================

;* The following is the original TI-83 assembly code taken from the WikiTI
;* ([WTI.RAND]):
;* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;* ;-----> Generate a random number
;* ; output a=answer 0<=a<=255
;* ; all registers are preserved except: af
;* random:
;*         push    hl
;*         push    de
;*         ld      hl,(randData)
;*         ld      a,r
;*         ld      d,a
;*         ld      e,(hl)
;*         add     hl,de
;*         add     a,l
;*         xor     h
;*         ld      (randData),hl
;*         pop     de
;*         pop     hl
;*         ret
;* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;* "randData here must be a 2 byte seed located in ram. While this is a fast
;* generator, it's generally not considered very good in terms of randomness."
;* [WTI.RAND]

;* GBZ80 has no "ld a,r". "R is the dynamic RAM refresh register, it increases
;* after every instruction by an amount depending on the instruction. Its
;* contents are pseudo random." [WB.TIREG] This means that we can replace it
;* with the divider register value.

;* =============================================================================
;* INCLUDES
;* =============================================================================

INCLUDE "HARDWARE.INC"
INCLUDE "GBRNG.INC"


;* =============================================================================
;* INITIALIZATION
;* =============================================================================

SECTION "RNG", ROM0

rand_init::
    ret                         ; 1|4


;* =============================================================================
;* RANDOM NUMBER GENERATOR
;* =============================================================================

rand::
    ld a, [GBRNG_RAMSEED]       ; 2|3   LDH
    ld h, a                     ; 1|1
    ld a, [GBRNG_RAMSEED+1]     ; 2|3   LDH
    ld l, a                     ; 1|1

    ld a, [rDIV]                ; 2|3   LDH

    ld d, a                     ; 1|1
    ld e, [hl]                  ; 1|2   $00 / $FF most of the time (see below)
    add hl, de                  ; 1|2
    add a, l                    ; 1|1
    xor a, h                    ; 1|1

    ld b, a                     ; 1|1
    ld a, h                     ; 1|1
    ld [GBRNG_RAMSEED], a       ; 2|3   LDH
    ld a, l                     ; 1|1
    ld [GBRNG_RAMSEED+1], a     ; 2|3   LDH
    ld a, b                     ; 1|1

    ret                         ; 1|4

                                ; 22|32 TOTAL (26|36 if ramseed in WRAM)

;* =============================================================================
;* REMARKS
;* =============================================================================

;* I am not familiar with the memory map of the TI-83 but this code is surely
;* not fit well for the GB. The "ld e, [hl]" instruction will load $00 or $FF
;* into E most of the time. Moreover, certain areas of the GB memory are marked
;* as not usable so it would be recommended to not even read in those locations.

;* This RNG seems too esoteric with its random memory address pick. It might be
;* tempting to choose exactly because of this silly attempt to be closer to true
;* randomness.

;* The tests I made show now reason yet to reject its use. However, I would not
;* recommend it until I can not do a full diehard test on the given stream. But
;* it would be hard to recommend such a weird RNG even with good diehard
;* results, as we can always suspect some flaws undetected.


;* =============================================================================
;* REFERENCES
;* =============================================================================

;* [WTI.RAND]   WiniTI: Z80 Routines:Math:Random
;*              http://wikiti.brandonw.net/index.php?title=Z80_Routines:Math:Random

;* [WB.TIREG]   TI 83 Plus Assembly/Registers
;               https://en.wikibooks.org/wiki/TI_83_Plus_Assembly/Registers
