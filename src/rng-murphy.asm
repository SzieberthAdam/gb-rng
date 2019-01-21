;* RNG-MURPHY
;* Copyright (c) 2019 Szieberth Ádám
;* 0BSD License (see LICENSE file for more info)


;* =============================================================================
;* ABSTRACT
;* =============================================================================

;* This is a very simple LCG with formula with parameters a=13, c=1, m=2⁸. It
;* is week but simple and fast. Due to nature of LCGs only a couple of higher
;* bits should be considered random.


;* =============================================================================
;* CHANGES
;* =============================================================================

;* I fully adapted the PIC 1802 assembly code listed on Donelly's website
;* ([DON.XORS]) to the GNZ80 CPU. B. J. Murphy's LCG is almost the same as the
;* RNG-Z80BITS8, only the multiplier changed from 5 to 13.


;* =============================================================================
;* INCLUDES
;* =============================================================================

INCLUDE "RNG.INC"


;* =============================================================================
;* INITIALIZATION
;* =============================================================================

SECTION "RNG", ROM0

rand_init::
    ret                         ; 1|4


;* =============================================================================
;* RANDOM NUMBER GENERATOR
;* =============================================================================

rand::                          ; 13|18 (15|20 if RNGSEED in WRAM)
    ld a, [RNGSEED]             ; 2|3   LDH; A = N
    ld b, a                     ; 1|1   B = N
    add a, a                    ; 1|1   A = 2 * N
    add a, a                    ; 1|1   A = 4 * N
    ld c, a                     ; 1|1   C = 4 * N
    add a, a                    ; 1|1   A = 8 * N
    add a, c                    ; 1|1   A = 12 * N
    add a, b                    ; 1|1   A = 13 * N
    inc a                       ; 1|1   A = 13 * N + 1
    ld [RNGSEED], a             ; 2|3   LDH
  ret                           ; 1|4


;* =============================================================================
;* REMARKS
;* =============================================================================

;* This simple LCG might be a good choice for games which can update the seed
;* regularly with the user inputs and require not too many small values as the
;* period of the RNG is 256 for the most significant bit and and 2 for the least
;* significant bit. Thus, some of the least significant bits should be surely
;* discarded.

;* If the game requires a random number after every user inputs, consider using
;* RNG-2048GB instead. However, if the game should be random but also
;* deterministic or speedrun friendly then this might fit to it.


;* =============================================================================
;* REFERENCES
;* =============================================================================

;* [DON.XORS]   William Donnelly: Unsigned 8-Bit XOR-Shift Pseudo-Random Number
;*              Generator Algorithm Test
;*              http://www.donnelly-house.net/programming/cdp1802/8bitPRNGtest.html

;* [Z80BITS8]   Z80 Bits: 8-bit Random Number Generator
;*              http://www.msxdev.com/sources/external/z80bits.html#3.1
