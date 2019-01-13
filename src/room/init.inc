INCLUDE "ROOM/UTIL.INC"


SECTION	"Initialize Display", ROM0

; Initializes the display. Draws the area of addresses and values but leaves
; the row area of the addresses and the value area empty.
; @param B; bit 0 should flag whether we draw empty spaces in the random map
;     (bit0=0) or the random values (bit0=1)
; @destroys AF, BC, DE, HL
clear_display::

    ld hl, $98C0                ; 3|4   set HL to destination address (BG map)

;* To follow which random value we should draw next we define a counter in
;* register C for it. Ideally it should be incremented after a value has been
;* drawn. However, we are determining whether we will draw on the random map
;* area prior to the actual draw and testing it thereafter would make us do the
;* same thing twice. This forces us to increment the counter prior to the draw.
;* Unfortunately we can not just set the counter to -1 as that would mess up the
;* address titles. At the time we render the address titles at the beginning of
;* a row the counter should be set to the fine value. So what we will do is to
;* decrement the counter after an address title has been rendered (specifically
;* when we draw the horizontal line tile) and that weill be incremented back
;* soon when the program knows that we stepped into the random map area. That
;* extra decrement should be compensated at the end of the rows. As we do that
;* generally and we have two rows header before reaching the random map area, we
;* should set our initial counter to -2.

    ld c, -2                    ; 2|2   C = value counter (will be incremented)

.repeat

;* Now we want to set the next tile into the B register. For that we want to
;* know the row and column values. Bits 4-0 of the L register contains the
;* column index. Bits 1-0 of H register and bits 7-5 of L register together
;* contain the row index plus 7 as we are vertically shifted by 7 tiles. A H
;* value of $98 indicates the first row and $9B indicates the last row.

;* We optimize our code for the most frequent scenario: being in the third or
;* higher row number and not in the third column.

    ld a, h                     ; 1|1
    cp a, $98                   ; 2|2
    jr z, .atrow0or1            ; 2|3/2

;* In the second row we expect to be at the value section which is in column
;* 5-20. Otherwise we jump to determine the address tiles and the vertical bar.

.atrow2plus
    ld a, l                     ; 1|1
    and a, %00011100            ; 2|2   4 <= column index
    jr z, .atrow2plustitle      ; 2|2/3
    inc c

.setrandomtile
    ld a, [RANDMAPDISP+1]       ; 2|3   LDH
    add a, c                    ; 1|1
    ld e, a                     ; 1|1
    ld a, [RANDMAPDISP]         ; 2|3
    adc a, 0                    ; 2|2
    ld d, a                     ; 1|1
    ld a, [de]                  ; 1|2
    ld d, a

;* As we write to the VRAM in less than 20 cycles after loading the status
;* register value, we can be sure that we have have access to it. We have access
;* to the VRAM during the 20 cycles long OAM Search which follows every blanking
;* period. Note that for this, OAM Search should be uninterrupted so LYC=LY and
;* Mode 2 interrupts should be disabled.

;* We write the tile into the RAM address and we also increment the RAM address
;* in the HL register.

.wait_for_vram
    ld a, [rSTAT]               ; 2|3
    and %00000010               ; 2|2   in mode 2 or 3?
    jr nz, .wait_for_vram       ; 2|2/3 if no, wait some more
    ld a, d                     ; 1|2   A = tile value
    ld [hl+], a                  ; 1|2   set tile

;* Now we want to know if we are at the end of a row. As mentioned above, we can
;* get the column index from the bits 4-0 of the L register. However, now we are
;* particularly interested in having a decimal value of 20 there. If we are not
;* at the end of a row, we can continue replacing the tiles.

.endrowtest
    ld a, l                     ; 1|1
    and a, 20                   ; 2|2
    cp a, 20                    ; 2|2
    jr nz, .repeat              ; 2|2/3

;* Here we are at the row end of a row which is not the last thus we want to
;* jump to the beginning of the next one in the BG map. For that we have to
;* increase HL by 12.

.atrowXend
    ld a, l                     ; 1|1
    add a, 12                   ; 2|2
    ld l, a                     ; 1|1
    ld a, h                     ; 1|1
    adc a, 0                    ; 2|2

;* If the start of the new line would be at $9B00, we are done.

    cp a, $9B                   ; 2|2
    jr z, .done                 ; 2|3/2

;* Otherwise we increment the value counter as mentioned above.

    ld h, a                     ; 1|1
    inc c                       ; 1|1   increment C at the end of every rows
    jr .repeat                  ; 2|3

.atrow0or1
    ld a, l                     ; 1|1
    and a, %00100000            ; 2|2
    jr nz, .atrow1              ; 2|3/2

.atrow0

;* In the first row we want the first four tiles to be empty spaces (32) and the
;* remaining tiles to be hexadecimal numbers from 0 to F.

    ld a, l                     ; 1|1
    and a, %00011111            ; 2|2
    sub a, 4                    ; 2|2
    jr c, .tile32               ; 2|2/3
    call hex_tile_of_a          ; 3|16/17
    ld d, a                     ; 1|1
    jr .wait_for_vram           ; 2|3

.atrow1

;* In the second row we want the first three tiles to be empty spaces (32), the
;* fourth tile to be an CP437 single line topleft corner (218) and the remaining
;* tiles to be horizontal lines (196).

    ld a, l                     ; 1|1
    and a, %00011111            ; 2|2
    sub a, 3                    ; 2|2
    jr c, .tile32               ; 2|2/3
    jr z, .tile218

;* The 5th and subsequent tiles of the second row are CP437 single horizontal
;* box drawing lines (196).

.tile196
    ld d, 196                   ; 2|2
    jr .wait_for_vram           ; 2|3

;* The 4th tile of the 3rd row is a CP437 single line topleft box drawing corner
;* (218).

.tile218
    ld d, 218                   ; 2|2
    jr .wait_for_vram           ; 2|3

.tile32                         ;       empty space tile
    ld d, 32                    ; 2|2
    jr .wait_for_vram           ; 2|3

;* Here we are in the title area of the 3rd and subsequent rows. For each row
;* this contains three hexadecimal tiles of the address of the shown random map
;* row (the low nibble of the low byte is shown in the column header) and a
;* vertical box drawing character in the fourth column.

.atrow2plustitle
    ld a, l                     ; 1|1
    and a, %00000011            ; 2|2
    jr z, .atrow2pluscol0       ; 2|2/3
    cp a, 3                     ; 2|2
    jr z, .atrow2pluscol3       ; 2|2/3
    and a, 2                    ; 2|2
    jr z, .atrow2pluscol2       ; 2|2/3

;* The second tiles of the 3rd and subsequent rows are the hexadecimal values
;* of the low nibble of the high byte of the random value map rows.

.atrow2pluscol1
    ld a, [RANDMAPDISP+1]       ; 2|3   LDH
    add a, c                    ; 1|1
    ld a, [RANDMAPDISP]         ; 2|3
    adc a, 0                    ; 2|2
    and a, $0F                  ; 2|2
    call hex_tile_of_a          ; 3|16/17
    ld d, a                     ; 1|1
    jr .wait_for_vram           ; 2|3

;* The third tiles of the 3rd and subsequent rows are the hexadecimal values
;* of the high nibble of the low byte of the random value map rows.

.atrow2pluscol2
    ld a, [RANDMAPDISP+1]       ; 2|3   LDH
    add a, c                    ; 1|1
    and a, $F0                  ; 2|2
    swap a                      ; 2|2
    call hex_tile_of_a          ; 3|16/17
    ld d, a                     ; 1|1
    jr .wait_for_vram           ; 2|3

;* As mentioned above, we decrement the value counter after we are done with
;* rendering the address title.

.atrow2pluscol3
    dec c                       ; 1|1

;* The 4th tiles of the 3rd and subsequent rows are CP437 single vertical box
;* drawing lines (179).

.tile179
    ld d, 179                   ; 2|2
    jr .wait_for_vram           ; 2|3

;* The first tiles of the 3rd and subsequent rows are the hexadecimal values
;* of the high nibble of the high byte of the random value map rows. The 16 bit
;* address RANDMAPDISP contains the first value of the random map to be
;* displayed at the top left corner of the value area. This should be increased
;* by the C register value to get the address of the random value to be
;* displayed next.

.atrow2pluscol0
    ld a, [RANDMAPDISP+1]       ; 2|3   LDH
    add a, c                    ; 1|1
    ld a, [RANDMAPDISP]         ; 2|3
    adc a, 0                    ; 2|2
    and a, $F0                  ; 2|2
    swap a                      ; 2|2
    call hex_tile_of_a          ; 3|16/17
    ld d, a                     ; 1|1
    jp .wait_for_vram           ; 3|4


.done
    ret                         ; 1|4
                                ; 89|?
