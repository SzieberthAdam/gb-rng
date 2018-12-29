;* RNG-NULL ASM file
;* Copyright (c) 2018 Szieberth Ádám
;* 0BSD License (see LICENSE file for more info)

;* TO BE USED WITH GB-RNG.


;* =============================================================================
;* ABSTRACT
;* =============================================================================

;* This file contains no random number generator. It simply yields the 256 bytes
;* of the random seed area prepared by the GB-RNG main file ($DF00--$DFFF). Once
;* done with that, it keeps yielding zeroes.


SECTION "RNG", ROM0

rand_init::
    ld hl, $DF00                ; 3|3
    ret                         ; 1|4

rand::
    ld a, h                     ; 1|1
    cp $E0                      ; 2|2   we passed $DFFF
    jr z, .yield_null           ; 2|2/3
.yield_bc
    ld a, [hl+]                 ; 1|2
    ret                         ; 1|4
.yield_null
    xor a, a                    ; 1|1
    ret                         ; 1|4
