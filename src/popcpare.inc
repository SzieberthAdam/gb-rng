;* POPCPARE.INC
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

;* Now we pop the section from the section stack as we will add the following
;* subroutine there.

POPS


; Extracts randomness of a buffer by collecting the popcount parities (least
; significant bits) of eight subsequent N length data chunks, each contributing
; a bit to the one byte result.
; @param hl: A pointer to the beginning of the target buffer
; @param bc: N (feed counter) in the 13 LS bits; K (bit counter) in the 3 MS
;   bits which can be set from 0 to 7 and it sets how many bits should be
;   considered done. Should be set to zero most of the time. Note that maximum N
;   is 2^13 = $1000 whihch is half of the WRAM size so you are unable to extract
;   the whole WRAM into one bit, the highest compression you can get in one step
;   is two bits with BC=$D000 (N=$1000, bitcounter=6).
; @destroy de
; @return a, d: the extracted byte
extract_byte_from_data:
.prepare_result_variable
    xor a, a                    ; 1|1   A = 0
.rept_with_next_bit             ;  |(8-K)×..
    ld d, a                     ; 1|1
.prepare_feed_counter           ;  |(8-K)×..
    push bc                     ; 1|4
    ld a, b                     ; 1|1   start preparing the feed counter
    and a, %00011111            ; 2|2   bottom 5 bits goes to the byte caounter
    ld b, a                     ; 1|1   now the bit counter is prepared
.rept_with_this_bit             ;  |(8-K)×N×..
    ld a, [hl+]                 ; 1|2   loads the next byte

;* The next part is by ax6 ([GBD.PPARC]). It "returns 0 or -1. and of course
;* you'll also get the result in the z flag." [GBD.PPARC]

.popcount_parity_of_byte_to_a   ;  |(8-K)×N×..
    ld e, a                     ; 1|1
    swap a                      ; 2|2
    xor a, e                    ; 1|1
    ld e, a                     ; 1|1
    rlca                        ; 1|1
    rlca                        ; 1|1
    xor a, e                    ; 1|1
    ld e, a                     ; 1|1
    rlca                        ; 1|1
    xor a, e                    ; 1|1

;* For perfectness I want the absolute value of A this time. However an "inc a"
;* which is cheaper could be just as good with the opposite parity.

                                ;  |(8-K)×N×..
    ld e, a                     ; 1|1   abs a
    xor a, a                    ; 1|1   ...
    sub a, e                    ; 1|1   ___

.adjust_global_popcount_parity  ;  |(8-K)×N×..
    xor a, d                    ; 1|1   adjust parity (add a, d \\ and a, 1)
    ld d, a                     ; 1|1   save parity
    dec bc                      ; 1|2   decrease feed counter
.feed_counter_zero_check
    ld a, c                     ; 1|(8-K)×N
    and a                       ; 1|(8-K)×N                     ; ≡ cp a, 0
    jr nz, .rept_with_this_bit  ; 2|3×(8-K)×N-(8-K)×(⌊(N-1)/256⌋+1)
    ld a, b                     ; 1|(8-K)×(⌊(N-1)/256⌋+1)
    and a                       ; 1|(8-K)×(⌊(N-1)/256⌋+1)        ; ≡ cp a, 0
    jr nz, .rept_with_this_bit  ; 2|3×(8-K)×(⌊(N-1)/256⌋+1)-(8-K)
.feed_counter_at_zero
.adjust_bit_counter             ;  |(8-K)×..
    pop bc                      ; 1|3
    ld a, b                     ; 1|1
    and a, %11100000            ; 2|2   retain bit counter bits only
    rrca                        ; 1|1   shift one to the right (bit0=0)
    swap a                      ; 2|2   A = bit counter
    inc a                       ; 1|1   A = number of bits extracted
    cp a, 8                     ; 2|2   test if we are done

    jr z, .extraction_done      ; 2|(8-K)×3-(7-K)
.prepare_bit_counter            ;  |(7-K)×..
    rlca                        ; 1|1   shift one to the left (bit7=0)
    swap a                      ; 2|2
    ld e, a                     ; 1|1   overwrite top three bits of B with A
    ld a, b                     ; 1|1   ...
    and a, %00011111            ; 2|2   ...
    or a, e                     ; 1|1   ...
    ld b, a                     ; 1|1   ___
.adjust_result_variable         ;  |(7-K)×..
    ld a, d                     ; 1|1
    rlca                        ; 1|1   shift one to the left (bit7=0)
.continue_with_next_bit         ;  |(7-K)×..
    jr .rept_with_next_bit      ; 2|3
.extraction_done
    ld a, d                     ; 1|1
    ret                         ; 1|4

                                ; 60|329+256×N+⌊(N-1)/256⌋×(8-K)×4-K×(42+32×N)
                                ;                                          TOTAL

;* =============================================================================
;* REFERENCES
;* =============================================================================

;* [WP.HW]      Wikipedia: Hamming weight
;*              https://en.wikipedia.org/wiki/Hamming_weight4


;* [GBD.PPAR]   ax6 about popcount parity @ gbdev
;*              https://discordapp.com/channels/303217943234215948/303217943234215948/528876463357820928

;* [GBD.PPARC]  ax6's popcount parity code @ gbdev
;*              https://discordapp.com/channels/303217943234215948/490065800225488916/530701954989686794
