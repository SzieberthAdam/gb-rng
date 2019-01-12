;* RNG-Z80BITS8
;* Copyright (c) 2019 Szieberth Ádám
;* 0BSD License (see LICENSE file for more info)


;* =============================================================================
;* ABSTRACT
;* =============================================================================

;* "This is a very simple linear congruential generator. The formula is
;*                    x[i + 1] = (5 * x[i] + 1) mod 256.
;* Its only advantage is small size and simplicity. Due to nature of such
;* generators only a couple of higher bits should be considered random."
;* [Z80BITS8]


;* =============================================================================
;* INCLUDES
;* =============================================================================

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
    ld a, [RNGSEED]             ; 2|3   LDH; A = N
    ld b, a                     ; 1|1   B = N
    add a, a                    ; 1|1   A = 2 * N
    add a, a                    ; 1|1   A = 4 * N
    add a, b                    ; 1|1   A = 5 * N
    inc a                       ; 1|1   A = 5 * N + 1 (alternatively "add a, 7")
    ld [RNGSEED], a             ; 2|3   LDH
  ret                           ; 1|4

                                ; 10|15  TOTAL (12|17 if RNGSEED in WRAM)

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

;* [Z80BITS8]   Z80 Bits: 8-bit Random Number Generator
;*              http://www.msxdev.com/sources/external/z80bits.html#3.1
