;* RNG-NULL
;* Copyright (c) 2018 Szieberth Ádám
;* 0BSD License (see LICENSE file for more info)


;* =============================================================================
;* ABSTRACT
;* =============================================================================

;* This file contains no random number generator. It simply yields the 256 bytes
;* of the random seed area prepared by the GB-RNG main file ($DF00--$DFFF). Once
;* done with that, it keeps yielding zeroes.


;* =============================================================================
;* INCLUDES
;* =============================================================================

;* HARDWARE.INC contains the 'Hardware Defines' for our program. This has
;* address location labels for all of the GameBoy Hardware I/O registers. We can
;* 'insert' this file into the present ASM file by using the assembler INCLUDE
;* command:
INCLUDE "GBRNG.INC"


;* =============================================================================
;* INITIALIZATION
;* =============================================================================

SECTION "RNG", ROM0

rand_init::
    ld hl, GBRNG_SEED_START     ; 3|3
    ret                         ; 1|4

;* =============================================================================
;* RANDOM NUMBER GENERATOR
;* =============================================================================

;* As stated above, this is not a random number generator. The title serves as a
;* sample.

rand::
    ld a, h                     ; 1|1
    cp GBRNG_SEED_STOP >> 8     ; 2|2
    jr nc, .yield_null          ; 2|2/3 NC: GBRNG_SEED_STOP_Hi <= H
    jr nz, .yield_bc            ; 2|2/3
    ld a, l                     ; 1|1
    cp GBRNG_SEED_STOP & $FF    ; 2|2
    jr nc, .yield_null          ; 2|2/3 NC: GBRNG_SEED_STOP <= HL
.yield_bc
    ld a, [hl+]                 ; 1|2
    ret                         ; 1|4
.yield_null
    xor a, a                    ; 1|1
    ret                         ; 1|4
