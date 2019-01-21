;* RANDMAP.INC
;* Copyright (c) 2019 Szieberth Ádám
;* 0BSD License (see LICENSE file for more info)

;* =============================================================================
;* ABSTRACT
;* =============================================================================

;* This file handles the Random Map Screen of the GB-RNG software.


;* =============================================================================
;* THE CODE
;* =============================================================================

IF !DEF(RANDMAP_INC)
RANDMAP_INC SET 1

INCLUDE "MACRO/CP437.INC"
INCLUDE "MACRO/LCD.INC"


;* As the Random Map Screen has a slave interrupt handler, it should be placed
;* to a fixed address defined in the GBRNG.INC file.
SECTION	"Random Map V-Blank Handler", ROM0[RANDMAPVBHADDR]

; The Random Map Screen V-Blank handler checks and handles pressed and/or hold
; UP, DOWN, and START buttons. UP and DOWN moves the Random Map viewport over
; the random values in the WRAM while START generates another batch of random
; values. A pressed UP/DOWN moves the viewport by one line while a holded button
; is initially delayed then moves the viewport continously with increasing
; speed.
randmap_vblankhandler::

;* First we update our key values in the memory, then we check if START button
;* is pressed. If yes, we reset the hold value in the memory and initiate a
;* random value generation. Otherwise we check for UP/DOWN key presses. If one
;* of them is pressed, we check whether that button is pressed previously so
;* that it can be considered as hold. If hold, we check how long it has been
;* hold by loading the `hold counter` value from the memory. Fortunately the
;* user can either press UP or DOWN but not both so we can have a single
;* definite `hold counter` which is simply capped at $FF.
    call key                    ; 3|6+53
    ld a, [KEYNEW]              ; 2|3   A = newly pressed keys
    ld b, a                     ; 1|1   B = newly pressed keys
    and a, %00001000            ; 2|2   NZ if start is pressed
    jr nz, .start_pressed       ; 2|3/2
    ld a, b                     ; 1|1   A = newly pressed keys
    and a, %11000000            ; 2|2   A = newly pressed UP/DOWN keys
    jr z, .done                 ; 2|3/2 if none of those were pressed, done
    ld b, a                     ; 1|1   B = newly pressed UP/DOWN keys
    ld a, [KEYOLD]              ; 2|3   A = previously pressed keys
    and a, b                    ; 1|1   A = holded UP/DOWN keys
    jr z, .nohold               ; 2|3/2
    ld a, [KEYHOLD]             ; 2|3   A = hold counter
    inc a                       ; 1|1   Z set if hold counter overflow
    jr nz, .holdok              ; 2|3/2
    dec a                       ; 1|1   undo increment if overflow
.holdok
    ld [KEYHOLD], a             ; 2|3   save new hold counter in memory
    ld c, a                     ; 1|1   C = hold counter

;* At this point the user holds either UP or DOWN and we know that since how
;* frames he/she does that. Now we want to determine the offset which will be
;* applied to the Random Map viewport and that offset must be a multiply of 16.
;* Moreover, if the user holds the button for only a short time, then we do not
;* want to move the viewport beyond the one line move which was done when he/she
;* initially pressed the button. However, if the user holds the button for 16
;* frames or more, then we want to move the viewport the longer he/she holds the
;* button, the faster. For hold of 128 frames or more we want to have and offset
;* of 128, for a hold of 64 frames or more, we ant to have an offset of 64, etc.

;* Thus, our viewport offset value is the number of the most significant bit set
;* in the hold counter. Modern CPUs have a BSR (Bit Scan Reverse) opcode for
;* that but not the Game Boy CPU. So what we do is to shift the hold counter to
;* the left until we find the first bit set. It will reset the CY flag.
;* Previously we increased the hold counter so it is certainly not zero so that
;* CY flag will be set sooner or later. To determine the MSB we simultaneously
;* rotate a single high bit in a byte which have all other bits in low state so
;* that once we get the CY flag from the shifts it should contain the number of
;* the MSB in the hold counter. To be able to do the whole process with only one
;* jump instruction we set the offset byte to 1 and as part of the loop, we
;* immediately rotate that bit to the right so that it will be %10000000 for the
;* first shift of the hold register. If the hold register is %1xxxxxxx then we
;* will get the CY flag and we break the loop. Otherwise we move the offset high
;* bit to the right and then check the next bit of the hold counter from its
;* left and so on. Finally we end up with the number of the MSB of the hold
;* counter in the A register.
.bsr                            ;       Bit Scan Reverse
    ld a, %00000001             ; 2|2   A = 1; LSB set
._bsrloop
    rrca                        ; 1|1   initially A=128,MSB set; then Rshifted
    sla c                       ; 2|2   Lshift C; CY = most significant bit
    jr nc, ._bsrloop            ; 2|3/2
.checkholdperiod
    and a, %11110000            ; 2|2   Z set if holded in less than 16 frames
    jr z, .done                 ; 2|3/2 we do not react for short holds
    ld c, a                     ; 1|1   C = MSB of hold counter / offset value

;* Now that we know the offset value we want to also know whether we should move
;* the viewport up or down. Both ways we may require the current Display Offset
;* (Random Map Viewport Start Address) value multiple times so wwe load that to
;* HL and we will also pass its finel value in HL. Note that at this point the
;* user must pressed UP or DOWN. As we will see that the DOWN case took a little
;* more cycles, we set the jump instruction so that the action points to the UP
;* case which decreases the disbalance a little (by 1 cycle).
.holdcounterok
    ld a, [DISPOFFSET]          ; 2|3   A = Display Offset high byte
    ld h, a                     ; 2|3   H = Display Offset high byte
    ld a, [DISPOFFSET+1]        ; 2|3   A = Display Offset low byte
    ld l, a                     ; 2|3   L = Display Offset high byte
    bit 6, b                    ; 2|2   NZ: down
    jr nz, .go_up               ; 2|3/2

;* By going down, our viewport start address can not go above $DF00 as the
;* Random Map size is 256 so the bottom right value will be $DFFF in that case.
;* This means that if the high byte of the Display Offset is $DF we do not
;* permit going down more and that is known before anything else is checked.
;* Otherwise we add the offset value to the Display Offset.
.go_down
    ld a, h                     ; 1|1   A = Display Offset high byte
    cp a, $DF                   ; 2|2   Z if A = $DF
    jr z, .setzerodispofflb     ; 2|3/2
    ld a, l                     ; 2|3   A = Display Offset low byte
    add a, c                    ; 1|1
    ld l, a                     ; 1|1   L = new Display Offset low byte
    jr nc, .go_resolved         ; 2|3/2
    inc h                       ; 1|1
    jr .go_resolved             ; 2|3

;* By going up, our viewport start address can not go below $C000. However, we
;* might easily go up more with a $C0 high byte. For example if our Display
;* Offset is $C020 and the user presses the up button, the the new Display
;* Offset will be $C010. So we have to decrease the offset value from the
;* Display Offset low byte, and prevent the viewport move only if carry flag is
;* set and also the high byte is $C0. This looks like more complicated for the
;* first but funnily this requires less code as we can start working with the
;* Display Offset low byte immediately as it is in A at this point.
.go_up
    sub a, c                    ; 1|1
    ld l, a                     ; 1|1   L = new Display Offset low byte
    jr nc, .go_resolved         ; 2|3/2
    ld a, h                     ; 1|1   A = Display Offset high byte
    cp a, $C0                   ; 2|2   Z if A = $C0
    jr z, .setzerodispofflb     ; 2|3/2
    dec h                       ; 1|1
    jr .go_resolved             ; 2|3

;* As the above line is certainly jumps, this is a good place for a separate
;* sublabel so we put the code which deals with the situation if the user
;* presses the START button. This case we ensure the hold counter in the memory
;* is reset then we set next application mode to 2 which represents generation
;* of the next batch of random numbers. Finally we leave.
.start_pressed
    xor a, a                    ; 1|1   A = 0
    ld [KEYHOLD], a             ; 2|3/2 reset hold counter in memory
    ld a, 2                     ; 2|2   a = 2 (next app mode/state: generate)
    ld [MODE], a                ; 2|3   save next mode
    jr .post_any_pressed_mode   ; 2|3

;* As the above line is certainly jumps, this is a good place for a separate
;* sublabel so we put the code which deals with the situation if initial UP/DOWN
;* keypresses. As we said, we want to move the viewport by only line in these
;* cases which means that the offset should be 16. We could have put this before
;* `.holdcounterok` to which it finally jumps to but then the hold scenario
;* would have to jump which is not desired as that is the longer scenario by its
;* own.
.nohold
    xor a, a                    ; 1|1   A = 0
    ld [KEYHOLD], a             ; 2|3/2 reset hold counter in memory
    ld c, 16                    ; 2|2   shift value is 16 for a pressed UP/DOWN
    jr .holdcounterok           ; 2|3

;* This sublabel is also a jump target for cases where the Display Offset high
;* byte is set in H and we only require the low byte to be set to zero. Note
;* that above is a certain jump.
.setzerodispofflb
    ld l, 0                     ; 2|2   L = 0 = new Display Offset low byte

;* Now the new Display Offset value is in HL and we want to save it in the
;* memory. After that we set the next application mode to 1 which represents
;* refreshing the random map screen.
.go_resolved
    ld a, l                     ; 1|1   A = new Display Offset low byte
    ld [DISPOFFSET+1], a        ; 2|3
    ld a, h                     ; 1|1
    ld [DISPOFFSET], a          ; 2|3
    ld a, 1                     ; 2|2   a = 1 (next app mode/state: randmap)
    ld [MODE], a                ; 2|3   save next mode

;* If we handled any keypress event then we must changed the next application
;* mode so we can surely unset this V-Blank handler now.
.post_any_pressed_mode
    UnsetVblankHandler          ; 4|5

;* Finally we put the required teardown code to the end of the slave V-Blank
;* handler.
.done
    EndOfVblankHandler          ; 5|16


SECTION	"Random Map Screen", ROM0

; This subroutine initializes the Random Map Screen.
randmap_init:
    ld a, $C0                   ; 2|2
    ld [DISPOFFSET], a          ; 2|3
    xor a, a                    ; 1|1   ≡ ld a, 0
    ld [DISPOFFSET+1], a        ; 2|3
    ret                         ; 1|4


; This subroutine draws the first row of the Random Map Screen. The first four
; tiles there should be empty spaces (32) and the remaining tiles to be
; hexadecimal numbers from 0 to F.
randmap_row0:
    ld hl, $98CC                ; 3|4   start tile address
    ld b, 32                    ; 2|2   B = space tile
.repeat
    WaitForAnyBlank             ; 6|?
    ld a, b                     ; 1|1   A = tile
    ld [hl+], a                 ; 1|2   write tile to BG map
    ld a, l                     ; 1|1   at $98E0 we are done
    cp a, $E0                   ; 2|2   ...
    ret z                       ; 1|5/2 ___
    ld a, l                     ; 1|1
    and a, %00011111            ; 2|2   A = column
    sub a, $D0 & %00011111      ; 2|2   A = 0 should be at $98D0
    jr c, .repeat               ; 2|3/2 if A < 0 -> B remains space tile
    GetHexadecimalTile          ; 8|8/7
    ld b, a                     ; 1|1   B = hexadecimal tile
    jr .repeat                  ; 2|3


; This subroutine draws the second row of the Random Map Screen. The first
; three tiles there should be empty spaces (32), the fourth tile should be an
; CP437 single line topleft corner (218) and the remaining tiles should be
; horizontal box drawing lines (196).
randmap_row1:
    ld hl, $98EC                ; 3|4   start tile address
    ld b, 32                    ; 2|2   B = space tile
.repeat
    WaitForAnyBlank             ; 6|?
    ld a, b                     ; 1|1   A = tile
    ld [hl+], a                 ; 1|2   write tile to BG map
    ld a, l                     ; 1|1   at $9900 we are done
    and a, a                    ; 1|1   ...; ≡ cp a, 0
    ret z                       ; 1|5/2 ___
    ld a, l                     ; 1|1
    sub a, $EF                  ; 2|2   are we at $98EF?
    jr c, .repeat               ; 2|3/2 if before -> B remains space tile
    jr z, .tile218              ; 2|3/2 if yes -> corner tile
    ld b, 196                   ; 2|2   if after -> B = hline tile
    jr .repeat                  ; 2|3
.tile218
    ld b, 218                   ; 2|2   B = corner tile
    jr .repeat                  ; 2|3


; This subroutine draws the first column of the row addresses of the Random Map
; Screen. These are the high nibbles of the high bytes of the addresses of the
; random values in the appropriate row of the random map. That address increases
; by 16 with every row: address = [DISPOFFSET] + 16 * randmaprow. We render the
; tiles going downwards vertically.
randmap_addrcol0:
    ld hl, $990C                ; 3|4   start tile address
    ld c, 0                     ; 2|2   C = 16 * randmaprow
.repeat
    ld a, [DISPOFFSET+1]        ; 2|3   A = topleft address low byte
    add a, c                    ; 1|1   A = current address low byte
    ld a, [DISPOFFSET]          ; 2|3   A = topleft address high byte
    adc a, 0                    ; 2|2   A = current address high byte (AHB)
    and a, $F0                  ; 2|2   trim the low nibble
    swap a                      ; 2|2   A = AHB high nibble
    GetHexadecimalTile          ; 8|8/7
    ld b, a                     ; 1|1   B = hexadecimal tile
    WaitForAnyBlank             ; 6|?
    ld a, b                     ; 1|1   A = hexadecimal tile
    ld [hl], a                  ; 1|2   write tile to BG map
    ld a, l                     ; 1|1   HL += 32
    add a, 32 % 256             ; 2|2   ...
    ld l, a                     ; 1|1   ...
    ld a, h                     ; 1|1   ...
    adc a, 32 / 256             ; 2|2   ...
    ld h, a                     ; 1|1   ___
    cp a, $9B                   ; 2|2   at $9B0C we are done
    ret z                       ; 1|5/2 ___
    ld a, c                     ; 1|1   C += 16
    add a, 16                   ; 2|2   ...
    ld c, a                     ; 1|1   ___
    jr .repeat                  ; 2|3


; This subroutine draws the second column of the row addresses of the Random Map
; Screen. These are the low nibbles of the high bytes of the addresses of the
; random values in the appropriate row of the random map. That address increases
; by 16 with every row: address = [DISPOFFSET] + 16 * randmaprow. We render the
; tiles going downwards vertically.
randmap_addrcol1:
    ld hl, $990D                ; 3|4   start tile address
    ld c, 0                     ; 2|2   C = 16 * randmaprow
.repeat
    ld a, [DISPOFFSET+1]        ; 2|3   A = topleft address low byte
    add a, c                    ; 1|1   A = current address low byte
    ld a, [DISPOFFSET]          ; 2|3   A = topleft address high byte
    adc a, 0                    ; 2|2   A = current address high byte (AHB)
    and a, $0F                  ; 2|2   A = AHB low nibble
    GetHexadecimalTile          ; 8|8/7
    ld b, a                     ; 1|1   B = hexadecimal tile
    WaitForAnyBlank             ; 6|?
    ld a, b                     ; 1|1   A = hexadecimal tile
    ld [hl], a                  ; 1|2   write tile to BG map
    ld a, l                     ; 1|1   HL += 32
    add a, 32 % 256             ; 2|2   ...
    ld l, a                     ; 1|1   ...
    ld a, h                     ; 1|1   ...
    adc a, 32 / 256             ; 2|2   ...
    ld h, a                     ; 1|1   ___
    cp a, $9B                   ; 2|2   at $9B0D we are done
    ret z                       ; 1|5/2 ___
    ld a, c                     ; 1|1   C += 16
    add a, 16                   ; 2|2   ...
    ld c, a                     ; 1|1   ___
    jr .repeat                  ; 2|3


; This subroutine draws the third column of the row addresses of the Random Map
; Screen. These are the high nibbles of the low bytes of the addresses of the
; random values in the appropriate row of the random map. That address increases
; by 16 with every row: address = [DISPOFFSET] + 16 * randmaprow. We render the
; tiles going downwards vertically.
randmap_addrcol2:
    ld hl, $990E                ; 3|4   start tile address
    ld c, 0                     ; 2|2   C = 16 * randmaprow
.repeat
    ld a, [DISPOFFSET+1]        ; 2|3   A = topleft address low byte
    add a, c                    ; 1|1   A = current address low byte (ALB)
    and a, $F0                  ; 2|2   trim the low nibble
    swap a                      ; 2|2   A = ALB high nibble
    GetHexadecimalTile          ; 8|8/7
    ld b, a                     ; 1|1
    WaitForAnyBlank             ; 6|?
    ld a, b                     ; 1|1   A = hexadecimal tile
    ld [hl], a                  ; 1|2   write tile to BG map
    ld a, l                     ; 1|1   HL += 32
    add a, 32 % 256             ; 2|2   ...
    ld l, a                     ; 1|1   ...
    ld a, h                     ; 1|1   ...
    adc a, 32 / 256             ; 2|2   ...
    ld h, a                     ; 1|1   ___
    cp a, $9B                   ; 2|2   at $9B0E we are done
    ret z                       ; 1|5/2 ___
    ld a, c                     ; 1|1   C += 16
    add a, 16                   ; 2|2   ...
    ld c, a                     ; 1|1   ___
    jr .repeat                  ; 2|3


; This subroutine draws the fourth column of the row addresses of the Random Map
; Screen. These are CP437 single vertical box drawing lines (179).
randmap_addrcol3:
    ld hl, $990F                ; 3|4   start tile address
    ld b, 179                   ; 2|2   B = vertical line tile
    ld c, 0                     ; 2|2   C = 16 * randmaprow
.repeat
    WaitForAnyBlank             ; 6|?
    ld a, b                     ; 1|1   A = vertical line tile
    ld [hl], a                  ; 1|2   write tile to BG map
    ld a, l                     ; 1|1   HL += 32
    add a, 32 % 256             ; 2|2   ...
    ld l, a                     ; 1|1   ...
    ld a, h                     ; 1|1   ...
    adc a, 32 / 256             ; 2|2   ...
    ld h, a                     ; 1|1   ___
    cp a, $9B                   ; 2|2   at $9B0F we are done
    ret z                       ; 1|5/2 ___
    ld a, c                     ; 1|1   C += 16
    add a, 16                   ; 2|2   ...
    ld c, a                     ; 1|1   ___
    jr .repeat                  ; 2|3


; This subroutine clears the random map area of the Random Map Screen which is
; the bottom right 16*16 tiles of the screen. As we are going to display 256
; empty space (32) values, a single byte counter (and offset) will serve us
; well.
clear_randmap_vals:
    ld hl, $9910                ; 3|4   start address
    ld b, 32                    ; 2|2   B = space tile
    ld c, 0                     ; 2|2   C = value counter/offset
.repeat
    WaitForAnyBlank             ; 6|?
    ld a, b                     ; 1|1   A = space tile
    ld [hl+], a                 ; 1|2   write space tile to BG map
    inc c                       ; 1|1   C += 1
    ret z                       ; 1|5/2 if C overflowed, we are done
    ld a, c                     ; 1|1   A = value counter/offset
    and a, $0F                  ; 1|1   if the low nibble is nonzero then we are
    jr nz, .repeat              ; 2|3     not at the start of the next row
    ld a, l                     ; 1|1   otherwise, HL += 16
    add a, 16 % 256             ; 2|2   ...
    ld l, a                     ; 1|1   ...
    ld a, h                     ; 1|1   ...
    adc a, 16 / 256             ; 2|2   ...
    ld h, a                     ; 1|1   ___
    jr .repeat                  ; 2|3

;* The `randmap_val_blank` draws a single random value onto the Random Map Area
;* during a blanking period. It takes the 8 bit index value (`I`), get the
;* appropriate random value in the WRAM, calculates the appropriate BG map
;* address, then writes the value there. Getting the random value is easier as
;* it is in the memory address `DISPOFFSET+I`. The 16x16 Random Map's top left
;* corner's address in the BG map is $9910 which is the tile for `I=0`. Let's
;* see the BG map addresses of the various indices:
;*
;*                 | xI |    bI    | xAdr |       bAdr        |
;*                 |----|----------|------|-------------------|
;*                 | 00 | 00000000 | 9910 | 10011001 00010000 |
;*                 | 0F | 00001111 | 991F | 10011001 00011111 |
;*                 | 10 | 00010000 | 9930 | 10011001 00110000 |
;*                 | 1F | 00011111 | 993F | 10011001 00111111 |
;*                 |                   ....                   |
;*                 | 70 | 01110000 | 99F0 | 10011001 11110000 |
;*                 | 7F | 01111111 | 99FF | 10011001 11111111 |
;*                 | 80 | 10000000 | 9A10 | 10011010 00010000 |
;*                 | 8F | 10001111 | 9A1F | 10011010 00011111 |
;*                 |                   ....                   |
;*
;* As you can see the low nibble of the index is the low nibble of the address'
;* low byte which is fine. The high nibble though is splitted: The most
;* significant bit goes to the high byte of the address. More precisely it if it
;* is set then we add one to the initial high byte value which is $99. Bits 4-6
;* inclusive goes to the BG mapp address' low byte, to bits 5-7, respectively.
;* Bit 4 of the address' low byte is always set.

; This subroutine draws a single random value onto the Random Map Area during
; a blanking period.
; @param B the index of the value
; @destroy DE, HL
randmap_val_blank::             ; 29|36
    ld a, b                     ; 1|1   A = I
    and a, %11110000            ; 2|2   A = I high nibble
    set 3, a                    ; 2|2   set bit 4 here before the shift
    sla a                       ; 2|2   shift A to the left, CY = top bit
    ld l, a                     ; 1|1   L = unfinished address low byte
    ld a, $99                   ; 2|2   A = initial high byte
    adc a, 0                    ; 2|2   A = initial high byte + CY = high byte
    ld h, a                     ; 1|1   H = BG map address high byte
    ld a, b                     ; 1|1   A = I
    and a, %00001111            ; 1|1   A = I low byte
    add a, l                    ; 1|1   A = BG map address high byte
    ld l, a                     ; 1|1   L = BG map address high byte
    ld a, [DISPOFFSET+1]        ; 2|3   A = topleft randval address low byte
    add a, b                    ; 1|1   A = current randval address low byte
    ld e, a                     ; 1|1   E = current randval address low byte
    ld a, [DISPOFFSET]          ; 2|3   A = topleft randval address high byte
    adc a, 0                    ; 2|2   A = current randval address high byte
    ld d, a                     ; 1|1   D = current randval address high byte
    ld a, [de]                  ; 1|2   A = random value
    ld [hl], a                  ; 1|2   [HL] = random value (BG map)
    ret                         ; 1|4


; This subroutine draws the random map area of the Random Map Screen which is
; the bottom right 16*16 tiles of the screen. As we are going to display 256
; values, a single byte counter (and offset) will serve us well.
randmap_vals:
    ld hl, $9910                ; 3|4   start address
    ld c, 0                     ; 2|2   C = value counter/offset
.repeat
    ld a, [DISPOFFSET+1]        ; 2|3   A = topleft address low byte
    add a, c                    ; 1|1   A = current address low byte
    ld e, a                     ; 1|1   E = current address low byte
    ld a, [DISPOFFSET]          ; 2|3   A = topleft address high byte
    adc a, 0                    ; 2|2   A = current address high byte
    ld d, a                     ; 1|1   D = current address high byte
    ld a, [de]                  ; 1|2   A = random value
    ld b, a                     ; 1|1   B = random value
    WaitForAnyBlank             ; 6|?
    ld a, b                     ; 1|1   A = random value
    ld [hl+], a                 ; 1|2   write random value tile to BG map
    inc c                       ; 1|1   C += 1
    ret z                       ; 1|5/2 if C overflowed, we are done
    ld a, c                     ; 1|1   A = value counter/offset
    and a, $0F                  ; 1|1   if the low nibble is nonzero then we are
    jr nz, .repeat              ; 2|3     not at the start of the next row
    ld a, l                     ; 1|1   otherwise, HL += 16
    add a, 16 % 256             ; 2|2   ...
    ld l, a                     ; 1|1   ...
    ld a, h                     ; 1|1   ...
    adc a, 16 / 256             ; 2|2   ...
    ld h, a                     ; 1|1   ___
    jr .repeat                  ; 2|3


; This subroutine draws a new clear Random Map Screen.
randmap_new:
    call randmap_init
    call randmap_row0
    call randmap_row1
    call randmap_addrcol0
    call randmap_addrcol1
    call randmap_addrcol2
    call randmap_addrcol3
    call clear_randmap_vals
    ret                         ; 1|4

ENDC; RANDMAP_INC
