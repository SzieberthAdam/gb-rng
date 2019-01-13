SECTION	"display rand", ROM0


TileOfNum:
    cp a, 10
    jr c, .TileOfNum_carry
    add a, 7
.TileOfNum_carry
    add a, $30
    ret

room_init::
    ld hl, $9800                ; 3|4   set HL to destination address
    xor a, a
    ld b, a                     ;       row
    ld c, a                     ;       column
    ld e, $20                   ;       element
.repeat
.wait_for_vram
    ld a, [rSTAT]               ; 3|4
    and %00000010               ; 2|2   in mode 2 or 3
    jr nz, .wait_for_vram       ; 2|2/3
    ld a, e                     ; 1|2   get random byte
    ld [hl+], a                 ; 1|2   set random byte to VRAM address
.adjust_rc
    ld a, c
    cp a, 19
    jr nz, .no_cr
    ld c, $FF                   ;       will be incremented
    ld a, b
    cp a, 17
    jr z, .done
    inc b
    ld a, l                     ; 1|1
    add a, 12                   ; 2|2
    ld l, a                     ; 1|1
    ld a, h
    adc a, 0
    ld h, a
.no_cr
    inc c
.adjust_e
    ld a, b
    cp a, 0                     ;       TOTO
    jr z, .first_row
    cp a, 1
    jr z, .second_row
.thirdplus_row
    ld a, c
    cp a, 3
    jr z, .thirdplus_row_bar
    ld e, $20
    jr .repeat                  ; 2|3
.thirdplus_row_bar
    ld e, $B3
    jr .repeat                  ; 2|3
.first_row
    ld a, c
    sub a, 4
    jr c, .repeat
    call TileOfNum
    ld e, a
    jr .repeat                  ; 2|3
.second_row
    ld a, c
    cp a, 0
    jr z, .second_row_start
    cp a, 3
    jr z, .second_row_corner
    cp a, 4
    jr nz, .repeat
    ld e, $C4                   ; -
    jr .repeat                  ; 2|3
.second_row_start
    ld e, $20
    jr .repeat                  ; 2|3
.second_row_corner
    ld e, $DA
    jr .repeat                  ; 2|3

.done
    ret

;* Display 20×18 random numbers
;* -----------------------------------------------------------------------------

;* Now we display the random values. For this, the upper left 20×18 tiles of the
;* BG Map area of the VRAM has to get filled with the random bytes. However, as
;* we are not guaranteed that our RNG subroutine provides the value within a
;* handful of clocks, we should get the value before the STAT check.

;* We also should keep the first 20 tiles and skip the remaining 12 tiles of
;* every lines in the (32×32) BG map. The first tile to be skipped is at $9814
;* (note that $14==20). We simply increase the address register here by 12 and
;* go for the second row. The first tile to be skipped there is at $9834
;* ($34=20+12+20), and same for the third row and $9054 and so on. The LS bytes
;* are $14, $34, $54, $74, $94, $B4, $D4, $F4, and again, $14. The lower 5 bits
;* of these numbers are identical: %10100=$14. We will AND our address with this
;* value to determine whether we shoudl jump to the next line.

;* At $9A34 we are done or if we do the line jump first then at $9A40. We could
;* use a row counter instead of the 2 bytes address comparision but then we
;* should use here as few registers as possible which makes them available for
;* the RNG subroutine without the need of push/pop. So this code uses the B
;* register to cache the random byte and the HL register for the VRAM address.

;display_random_numbers::
;    ld bc, GBRNG_RES_START      ; 3|4   set BC to source (random area) address
;    ld hl, $9800                ; 3|4   set HL to destination address
;.repeat
;.wait_for_vram
;    ld a, [rSTAT]               ; 3|4
;    and %00000010               ; 2|2   in mode 2 or 3
;    jr nz, .wait_for_vram       ; 2|2/3
;    ld a, [bc]                  ; 1|2   get random byte
;    ld [hl+], a                 ; 1|2   set random byte to VRAM address
;    inc bc                      ; 1|2
;.end_check                      ;       HL == $9A34; .done; .end_check_break
;    ld a, h                     ; 1|1
;    cp $9A                      ; 2|2
;    jr nz, .end_check_break     ; 2|2/3
;    ld a, l                     ; 1|1
;    cp $34                      ; 2|2
;    jr z, .done                 ; 2|2/3
;.end_check_break
;.skip12tiles_check              ;       L & $14 == $14; .skip12tiles; .repeat
;    ld a, l                     ; 1|1
;    and a, $14                  ; 2|2
;    cp a, $14                   ; 2|2
;    jr nz, .repeat              ; 2|2/3
;.skip12tiles
;    ld a, l                     ; 1|1
;    add a, 12                   ; 2|2
;    ld l, a                     ; 1|1
;    jr nc, .repeat              ; 2|2/3
;    inc h                       ; 1|1   there was a carry by the low nibble ADD
;    jr .repeat                  ; 2|3
;.done
;
;    ret


display_random_numbers::
    ld hl, $9840                ; 3|4   set HL to destination address
    xor a, a
    ld b, a                     ;       bytenr
    ld c, a                     ;       mode
.repeat
    ld a, c
    cp a, 0
    jr nz, .not_at_col0
.at_col0
    ld a, [GBRNG_DSP_CUR+1]
    add a, b
    ld a, [GBRNG_DSP_CUR]
    adc a, 0
    and a, $F0
    swap a
    call TileOfNum
    ld e, a
    jr .wait_for_vram
.not_at_col0
    cp a, 1
    jr nz, .not_at_col1
.at_col1
    ld a, [GBRNG_DSP_CUR+1]
    add a, b
    ld a, [GBRNG_DSP_CUR]
    adc a, 0
    and a, $0F
    call TileOfNum
    ld e, a
    jr .wait_for_vram
.not_at_col1
    cp a, 3
    jr z, .adjust_rc
    jr nc, .at_randnr
.at_col2
    ld a, [GBRNG_DSP_CUR+1]
    add a, b
    and a, $F0
    swap a
    call TileOfNum
    ld e, a
    jr .wait_for_vram
.at_randnr
    ld a, [GBRNG_DSP_CUR+1]
    add a, b
    ld e, a
    ld a, [GBRNG_DSP_CUR]
    adc a, 0
    ld d, a
    ld a, [de]
    ld e, a
.wait_for_vram
    ld a, [rSTAT]               ; 3|4
    and %00000010               ; 2|2   in mode 2 or 3
    jr nz, .wait_for_vram       ; 2|2/3
    ld a, e                     ; 1|2   get random byte
    ld [hl], a                  ; 1|2   set random byte to VRAM address
.adjust_rc
    inc hl
    inc c
    ld a, c
    cp 5
    jr c, .repeat
    inc b
    jr z, .done
    ld a, c
    cp a, 20
    jr nz, .repeat
    ld a, l                     ; 1|1
    add a, 12                   ; 2|2
    ld l, a                     ; 1|1
    ld a, h
    adc a, 0
    ld h, a
    xor a, a
    ld c, a
    jr .repeat                  ; 2|3
.done
    ret
