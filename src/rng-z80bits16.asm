;* RNG-Z80BITS16
;* Copyright (c) 2019 Szieberth Ádám


;* =============================================================================
;* ABSTRACT
;* =============================================================================

;* "This generator is based on similar method but gives much better results
;* (than RNG-Z80BITS8 -- SA). It was taken from an old ZX Spectrum game and
;* slightly optimised." [Z80BITS16]


;* =============================================================================
;* CHANGES
;* =============================================================================

;* The following is the original Z80 assembly code taken from the Z80 Bits page
;* ([Z80BITS16]):
;* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;* Rand16 ld de,Seed  ; Seed is usually 0
;*     ld a,d
;*     ld h,e
;*     ld l,253
;*     or a
;*     sbc hl,de
;*     sbc a,0
;*     sbc hl,de
;*     ld d,0
;*     sbc a,d
;*     ld e,a
;*     sbc hl,de
;*     jr nc,Rand
;*     inc hl
;* Rand ld (Rand16+1),hl
;*     ret
;* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

;* The GB CPU has no 16 bit SBC instruction so those has to get split. On top of
;* that, the splitted parts clear A so A has to get chached in some places. Also
;* the 16 bit values has to get loaded in two steps. Overall, these made the
;* above code big and slow on the Game Boy CPU. The "ld l, 253 // or a" lines
;* were replaced by "ld a, $FD // ld l, a // ccf" so that the value of L is
;* prepared in A for the 16 bit sbc. The "or a" in the original code have no
;* effect beyond clearing the carry flag.

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
    ld a, [GBRNG_RAMSEED+1]     ; 2|2   LDH
    ld e, a                     ; 1|1
    ld a, [GBRNG_RAMSEED]       ; 2|2   LDH
    ld d, a                     ; 1|1

    ld h, e                     ; 1|1

    ld a, $FD                   ; 2|2
    ld l, a                     ; 1|1
    ccf                         ; 1|1

    sbc a, e                    ; 1|1
    ld l, a                     ; 1|1
    ld a, h                     ; 1|1
    sbc a, d                    ; 1|1
    ld h, a                     ; 1|1

    ld a, d                     ; 1|1
    sbc a, 0                    ; 2|2
    ld b, a                     ; 1|1

    ld a, l                     ; 1|1
    sbc a, e                    ; 1|1
    ld l, a                     ; 1|1
    ld a, h                     ; 1|1
    sbc a, d                    ; 1|1
    ld h, a                     ; 1|1

    ld d, 0                     ; 2|2
    ld a, b                     ; 1|1
    sbc a, d                    ; 1|1
    ld e, a                     ; 1|1

    ld a, l                     ; 1|1
    sbc a, e                    ; 1|1
    ld l, a                     ; 1|1
    ld a, h                     ; 1|1
    sbc a, d                    ; 1|1
    ld h, a                     ; 1|1

    jr nc, .rand                ; 2|2/3
    inc hl                      ; 1|1
.rand
    ld a, h                     ; 1|1
    ld [GBRNG_RAMSEED], a       ; 2|2   LDH
    ld a, l                     ; 1|1
    ld [GBRNG_RAMSEED+1], a     ; 2|2   LDH
    ret                         ; 1|4

                                ; 47|50  TOTAL


;* =============================================================================
;* REMARKS
;* =============================================================================

;* I have concerns regarding this RNG. First of all, it seems too esoteric for
;* me. While LCG / LFSR / XORShift RNGs are well documented and tested on
;* scientific level, there is no source of this RNG. I see no reason picking
;* this one over a LCG, especially as it seems that it can not be optimized for
;* the GB CPU. 50 cycles seems too costly for me for what it does.

;* Note that GB-RNG takes one bytes per call from the A register which holds the
;* low byte of the returned 16bit random number. I intuitively picked the low
;* byte to have more entropy but I did not test it in any way.


;* =============================================================================
;* REFERENCES
;* =============================================================================

;* [Z80BITS16]  Z80 Bits: 16-bit Random Number Generator
;*              http://www.msxdev.com/sources/external/z80bits.html#3.2
