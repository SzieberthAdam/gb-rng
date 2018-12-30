;* RNG-ION
;* Copyright (c) 2018 Szieberth Ádám


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
;*
;* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;* "randData here must be a 2 byte seed located in ram. While this is a fast
;* generator, it's generally not considered very good in terms of randomness."
;* [WTI.RAND]

;* GBZ80 has no "ld a,r". "R is the dynamic RAM refresh register, it increases
;* after every instruction by an amount depending on the instruction. Its
;* contents are pseudo random." [WB.TIREG] This means that we can replace it
;* with the divider register value.

;* Because of the unique memory map of the Game Boy, I divided D by two which
;* ensures that the readed byte is taken from the ROM and will not access memory
;* locations marked as not usable.

;* =============================================================================
;* INCLUDES
;* =============================================================================

;* HARDWARE.INC contains the 'Hardware Defines' for our program. This has
;* address location labels for all of the GameBoy Hardware I/O registers. We can
;* 'insert' this file into the present ASM file by using the assembler INCLUDE
;* command:
INCLUDE "HARDWARE.INC"


;* =============================================================================
;* INITIALIZATION
;* =============================================================================

SECTION "RNG", ROM0

;* This RNG requires two bytes of random seed. We have 256 bytes at startup
;* starting from address $DF00 so we pick the forst two bytes.

randData EQU $FFC0

rand_init::
    ld a, [$DF00]               ; 3|4
    ld [randData], a            ; 2|3   LDH
    ld a, [$DF00 + 1]           ; 3|4
    ld [randData + 1], a        ; 2|3   LDH
    ret                         ; 1|4


;* =============================================================================
;* RANDOM NUMBER GENERATOR
;* =============================================================================

;* The seed value determines which value of the table below gets XOR'ed with the
;* divider register value. Thereafter the next value of the table is added to
;* the result and this new value is the random value produced and also the new
;* seed value.

rand::
    ld a, [randData]            ; 2|3   LDH
    ld h, a                     ; 1|1
    ld a, [randData + 1]        ; 2|3   LDH
    ld l, a                     ; 1|1

    ld a, [rDIV]                ; 2|3   LDH

    ld d, a                     ; 1|1
    srl d                       ; 3|3   added division of the high byte by 2
    ld e, [hl]                  ; 1|2   $00 / $FF most of the time (see below)
    add hl, de                  ; 1|2
    add a, l                    ; 1|1
    xor a, h                    ; 1|1

    ld b, a                     ; 1|1
    ld a, h                     ; 1|1
    ld [randData], a            ; 2|3   LDH
    ld a, l                     ; 1|1
    ld [randData + 1], a        ; 2|3   LDH
    ld a, b                     ; 1|1

    ret                         ; 1|4


;* =============================================================================
;* REMARKS
;* =============================================================================

;* I am not familiar with the memory map of the TI-83 but this code is surely
;* not fit for the GB. The "ld e, [hl]" instruction will load $00 or $FF into E
;* most of the time. Moreover, certain areas of the GB memory are marked as not
;* usable so it would be recommended to not even read in those locations.

;* Still, for the first look this RNG seems an acceptable one for a not
;* seriously RNG dependant game.

;* It seems that identical tiles tend to appear close to each other here and
;* there too often but. Might be acceptable.


;* =============================================================================
;* REFERENCES
;* =============================================================================

;* [WTI.RAND]   WiniTI: Z80 Routines:Math:Random
;*              http://wikiti.brandonw.net/index.php?title=Z80_Routines:Math:Random

;* [WB.TIREG]   TI 83 Plus Assembly/Registers
;               https://en.wikibooks.org/wiki/TI_83_Plus_Assembly/Registers
