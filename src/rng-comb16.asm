;* RNG-COMB16
;* Copyright (c) 2019 Szieberth Ádám
;* 0BSD License (see LICENSE file for more info)


;* =============================================================================
;* ABSTRACT
;* =============================================================================

;* "This is a very fast, quality pseudo-random number generator. It combines a
;* 16-bit Linear Feedback Shift Register and a 16-bit LCG." [WTI.RAND]


;* =============================================================================
;* CHANGES
;* =============================================================================

;* The following is the original TI-83 assembly code taken from the WikiTI
;* ([WTI.RAND]):
;* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;* prng16:
;* ;Inputs:
;* ;   (seed1) contains a 16-bit seed value
;* ;   (seed2) contains a NON-ZERO 16-bit seed value
;* ;Outputs:
;* ;   HL is the result
;* ;   BC is the result of the LCG, so not that great of quality
;* ;   DE is preserved
;* ;Destroys:
;* ;   AF
;* ;cycle: 4,294,901,760 (almost 4.3 billion)
;* ;160cc
;* ;26 bytes
;*     ld hl,(seed1)
;*     ld b,h
;*     ld c,l
;*     add hl,hl
;*     add hl,hl
;*     inc l
;*     add hl,bc
;*     ld (seed1),hl
;*     ld hl,(seed2)
;*     add hl,hl
;*     sbc a,a
;*     and %00101101
;*     xor l
;*     ld l,a
;*     ld (seed2),hl
;*     add hl,bc
;*     ret
;* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

;* 16 bit loads are adapted to GBZ80 which requires two separate 8 bit loads per
;* one 16 bit load. Naturally this caused the code to be bigger and slower than
;* on the TI. Thankfully the other parts of the code are compatible with the
;* Game Boy CPU.


;* =============================================================================
;* INCLUDES
;* =============================================================================

INCLUDE "HARDWARE.INC"
INCLUDE "RNG.INC"


;* =============================================================================
;* INITIALIZATION
;* =============================================================================

SECTION "RNG", ROM0

;* I have to ensure that the second 16 bit value is nonzero.

rand_init::
    ld hl, RNGSEED+2            ; 3|3
.seed34
    ld a, [hl+]                 ; 1|2
    and a, a                    ; 1|1
    jr nz, .done                ; 2|3/2
    ld a, [hl]                  ; 1|2
    and a, a                    ; 1|1
    jr nz, .done                ; 2|3/2
.repeat2                        ; 1|1
    ld a, [rDIV]                ; 3|4
    and a, a                    ; 1|1
    jr z, .repeat2              ; 2|3/2
    ld [hl], a                  ; 1|2
.done
    ret                         ; 1|4

;* =============================================================================
;* RANDOM NUMBER GENERATOR
;* =============================================================================

rand::                          ; 38|54 (46|62 if RNGSEED in WRAM)

;* The LCG is one of the simplest we can have:
;* X[n+1] = (5 * X[n] + 1) mod 65535

.lcg                            ;
    ld a, [RNGSEED]             ; 2|3   LDH
    ld h, a                     ; 1|1
    ld a, [RNGSEED+1]           ; 2|3   LDH
    ld l, a                     ; 1|1

    ld b, h                     ; 1|1
    ld c, l                     ; 1|1
    add hl, hl                  ; 1|2
    add hl, hl                  ; 1|2
    add hl, bc                  ; 1|2
    inc l                       ; 1|1

    ld a, l                     ; 1|1
    ld [RNGSEED+1], a           ; 2|3   LDH
    ld a, h                     ; 1|1
    ld [RNGSEED], a             ; 2|3   LDH

;* The LFSR ([NWI.LFSR]) is a Galois 16 bit (stages) right direction LFSR
;* ([W.GLFSR]) with polynomial X^16 + X^14 + X^13 + X^11 + 1 which has the $B400
;* left direction toggle mask. Our LFSR has right direction (MSB is the output)
;* so the toggle mask becomes $002D.

.lfsr
    ld a, [RNGSEED+2]           ; 2|3   LDH
    ld h, a                     ; 1|1
    ld a, [RNGSEED+3]           ; 2|3   LDH
    ld l, a                     ; 1|1

    add hl, hl                  ; 1|2   ≡ sla hl (output bit=MSB= -> carry flag)
    sbc a, a                    ; 1|1   A = $00/$FF if output bit is 0/1
    and a, $2D                  ; 2|2   A = $00/$2D if MSB is 0/1
    xor a, l                    ; 1|1   if output bit is 1, apply toggle mask
    ld l, a                     ; 1|1

    ld [RNGSEED+3], a           ; 2|3   LDH
    ld a, h                     ; 1|1
    ld [RNGSEED+2], a           ; 2|3   LDH

;* Now we add the result of the LCG to the LFSR. As BC holds the previous state
;* of the LCG, we can do that instantly without loads.

;* As GB-RNG expects an 8 bit random number in A, I load the high byte of the
;* result into A as due to the nature of the LCG, the high bytes have more
;* entropy. If you use this RNG and are fine with the 16 bit result in HL, just
;* comment out the "ld a, h" line.

.mixup

    add hl, bc                  ; 1|2   BC has the previous state of the LCG
    ld a, h                     ; 1|1   optionally put

    ret                         ; 1|4


;* Note that GB-RNG only takes the value from A.


;* =============================================================================
;* REMARKS
;* =============================================================================

;* "On their own, LCGs and LFSRs don't produce great results and are generally
;* very cyclical, but they are very fast to compute. The 16-bit LCG in the above
;* example will bounce around and reach each number from 0 to 65535, but the
;* lower bits are far more predictable than the upper bits. The LFSR mixes up
;* the predictability of a given bit's state, but it hits every number except 0,
;* meaning there is a slightly higher chance of any given bit in the result
;* being a 1 instead of a 0. It turns out that by adding together the outputs of
;* these two generators, we can lose the predictability of a bit's state, while
;* ensuring it has a 50% chance of being 0 or 1. As well, since the periods,
;* 65536 and 65535 are coprime, then the overall period of the generator is
;* 65535*65536, which is over 4 billion." [WTI.RAND]

;* This RNG has about the same size as the cc65 (LCG) but that LCG is a lot more
;* sophisticated than the one we use here. "The opinions on combination
;* generators are rather mixed. Marsaglia is a strong proponent of these types
;* of generators [...], on the other hand, states, 'Combination generators are
;* not recommended because they are slow - usually two to three times slower
;* than the component generators.' In a similar vein, Coddington [1992] states:
;* 'Although these mixed generators perform well in empirical tests, there is
;* little theoretical understanding of their behavior, and it is quite possible
;* that mixing two generators may introduce new defects of which we are unaware.
;* A good single generator may therefore be preferable to a mixed generator.'"
;* [RNGREC]


;* =============================================================================
;* REFERENCES
;* =============================================================================

;* [RNGREC]     C. N. Zeeb & P. J. Burns: Random Number Generator Recommendation
;*              https://pdfs.semanticscholar.org/feaf/6a5176e197ec7afbc6e56e1f9136b5f24aa4.pdf

;* [NWI.LFSR]   New Wawe Instruments: Linear Feedback Shift Registers
;*              https://web.archive.org/web/20160731025245/http://www.newwaveinstruments.com/resources/articles/m_sequence_linear_feedback_shift_register_lfsr.htm

;* [W.GLFSR]    Wikipedia: Linear Feedback Shift Register; Galois LFSRs
;*              https://en.wikipedia.org/w/index.php?title=Linear-feedback_shift_register&section=2#Galois_LFSRs

;* [WB.TIREG]   TI 83 Plus Assembly/Registers
;               https://en.wikibooks.org/wiki/TI_83_Plus_Assembly/Registers

;* [WTI.RAND]   WiniTI: Z80 Routines:Math:Random
;*              http://wikiti.brandonw.net/index.php?title=Z80_Routines:Math:Random
