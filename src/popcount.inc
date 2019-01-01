POPS

; Calculate the popcount of A
; @param a contains the byte whose popcount will be calculated
; @return a the calculated popcount value
; @return flags C reset, Z reset/set depending on the popcount
popcnt_of_a:

    push bc                     ; 1|4

    ld c, a                     ; 1|1
    and a, %01010101            ; 2|2   b0: every other bit from a
    ld b, a                     ; 1|1
    ld a, c                     ; 1|1
    srl a                       ; 2|2
    and a, %01010101            ; 2|2   b1: the remaining bits from a
    add a, b                    ; 1|1   c=b0+b1: list giving # of 1s in each
                                ;       2-bit slice of a

    ld c, a                     ; 1|1
    and a, %00110011            ; 2|2
    ld b, a                     ; 1|1
    ld a, c                     ; 1|1
    srl a                       ; 2|2
    srl a                       ; 2|2
    and a, %00110011            ; 2|2
    add a, b                    ; 1|1

    ld c, a                     ; 1|1
    and a, %00001111            ; 2|2
    ld b, a                     ; 1|1
    ld a, c                     ; 1|1
    swap a                      ; 2|2   srl a * 4
    and a, %00001111            ; 2|2   ___
    add a, b                    ; 1|1

    pop bc                      ; 1|3

    ret                         ; 1|4


; Calculate the popcount of data
; @param hl A pointer to the beginning of the target buffer
; @param bc length of the target data
; @return bc the calculated popcount value
; @return flags C reset, Z reset/set depending on the popcount
popcnt_of_data:
    push de                     ; 1|4   will store the popcount value
    ld d, 0                     ; 2|2   ...
    ld e, 0                     ; 2|2   ___
    inc b                       ; 1|1
    inc c                       ; 1|1
.repeat
.bc_compare
    dec c                       ; 1|1
    jr nz, .bc_nonzero          ; 2|2/3
    dec b                       ; 1|1
    jr nz, .bc_nonzero          ; 2|2/3
.bc_zero
    ld a, d                     ; 1|1   copy DE to BC
    ld b, a                     ; 1|1   ...
    ld a, e                     ; 1|1   ...
    ld c, a                     ; 1|1   ___
    pop de                      ; 1|3
    ret                         ; 1|4
.bc_nonzero
    ld a, [hl+]                 ; 1|2
    call popcnt_of_a            ; 3|6
    add a, e                    ; 1|1
    ld e, a                     ; 1|1
    ld a, d                     ; 1|1
    adc a, 0                    ; 2|2
    ld d, a                     ; 1|1
    jr .repeat                  ; 2|3


; Extracts randomness of a buffer by collecting the popcount parities (least
; significant bits) of eight N length chunks, each contributing a bit to the
; result.
; @param hl A pointer to the beginning of the target buffer
; @param bc the length of the chunks
; @return a the calculated popcount value
popcnt_extract_byte:
    push de                     ; 1|4
    ld d, 8                     ; 2|2   store the number of remaining bits
    ld e, 0                     ; 2|2   store the result value
.repeat
    push bc
    call popcnt_of_data         ; 3|6
    ld a, c
    rra
    ld a, e
    rla
    ld e, a
    pop bc
    dec d
    jr nz, .repeat
    ld a, e
    pop de                      ; 1|3
    ret                         ; 1|4


ExtractWRAMTo32bit: MACRO
    call popcnt_extract_byte
    ld [GBRNG_RAMSEED], a
    call popcnt_extract_byte
    ld [GBRNG_RAMSEED+1], a
    call popcnt_extract_byte
    ld [GBRNG_RAMSEED+2], a
    call popcnt_extract_byte
    ld [GBRNG_RAMSEED+3], a
ENDM


;* =============================================================================
;* REFERENCES
;* =============================================================================

;* [WP.HW]      Wikipedia: Hamming weight
;*              https://en.wikipedia.org/wiki/Hamming_weight
