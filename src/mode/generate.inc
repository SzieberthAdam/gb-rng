;* GENERATE.INC
;* Copyright (c) 2019 Szieberth Ádám
;* 0BSD License (see LICENSE file for more info)

;* =============================================================================
;* ABSTRACT
;* =============================================================================

;* This file handles the Random Number Generating Screen of the GB-RNG software.


; ******************************************************************************
; THE CODE
; ******************************************************************************

IF !DEF(GENERATE_INC)
GENERATE_INC SET 1

INCLUDE "GBRNG.INC"
INCLUDE "MACRO/ARITHMET.INC"
INCLUDE "MACRO/CP437.INC"
INCLUDE "MACRO/LCD.INC"
INCLUDE "MACRO/REGISTER.INC"

INCLUDE "MODE/RANDMAP.INC"

;* As the Generating Screen has a slave interrupt handler, it should be placed
;* to a fixed address defined in the GBRNG.INC file.
SECTION	"Generate V-Blank Handler", ROM0[GENERATEVBHADDR]

; The Generating Screen V-Blank handler does three things in decreasing
; priority: (1) updates the progress bar, (2) updates the address of the random
; number which is being calculated, and (3) renders the generated random values
; around the pop-up window onto the random map area.
generate_vblankhandler::

;* Our progress bar is 8 tiles wide and our addresses are in range $C000--$E000.
;* Thus, after every $400 a progress bar tile should be set. To make the
;* progress bar biased, we will set tiles at the following address high bytes:
;* $C2, $C6, $CA, $CE, $D2, $D6, $DA, $DE. Lets examine those binary values!
;*
;* $C0 11000000 nothing to do
;* $C1 11000001 nothing to do
;* $C2 11000010 1st progress bar tile; bit 1 indicates that; bit 2--4 index = 0
;* $C6 11000110 2nd; bit 1 set; bit 2--4 index = 1
;* $CA 11001010 3rd; bit 1 set; bit 2--4 index = 2
;* $CE 11001110 ...
;* $D2 11010010 ...
;* $D6 11010110 ...
;* $DA 11011010 ...
;* $DE 11011110 8th; bit 1 set; bit 2--4 index = 7
;* $DF 11011111 keep the 8th; bit 1 set; bit 2--4 index = 7
;* $E0 11100000 keep the 8th; bit 1 reset <-> nothing to do
;*
;* This means that we skip the progress bar update if bit 1 is reset and we get
;* our horizontal offset index from bits 2--4.
.update_progressbar             ; 20|23/9; 9 cycles when no progress tile to set
    LoadA GENERATEADDR          ; 2|3   A = high byte of the randval address
    ld b, a                     ; 1|1   B = high byte of the randval address
    and a, %00000010            ; 2|2   test for bit 1
    jr z, .update_randval_addr  ; 2|3/2
    ld a, b                     ; 1|1   A = high byte of the randval address
    and a, %00011100            ; 2|2   keep bit 2--4, offset index bits
    rrca                        ; 1|1   rotate right
    rrca                        ; 1|1   rotate right; A = progress bar offset
    ld hl, $99F2                ; 3|4   first progress bar tile BG map address
    add a, l                    ; 1|1   add the progress bar offset to L
    ld l, a                     ; 1|1   ___
    ld a, $DB                   ; 2|2   A = progress bar set tile
    ld [hl], a                  ; 1|2   write tile to the BG map

;* The 16bit random value address' four hexadecimal characters is shown centered
;* below the progress bar.
.update_randval_addr            ; 57|63/60
    ld hl, $9A14                ; 3|4   first randval addr. tile BG map address
    ld a, b                     ; 1|1   A = high byte of the randval addr. (AHB)
    and a, $F0                  ; 2|2   trim the low nibble
    swap a                      ; 2|2   A = AHB high nibble
    GetHexadecimalTile          ; 8|8   (always in A-F)
    ld [hl+], a                 ; 1|2   write tile to BG map; HL += 1
    ld a, b                     ; 1|1   A = AHB
    and a, $0F                  ; 2|2   A = AHB low nibble
    GetHexadecimalTile          ; 8|8/7
    ld [hl+], a                 ; 1|2   write tile to BG map; HL += 1
    LoadA GENERATEADDR+1        ; 2|3   A = low byte of the randval addr. (ALB)
    ld b, a                     ; 1|1   B = ALB
    and a, $F0                  ; 2|2   trim the low nibble
    swap a                      ; 2|2   A = ALB high nibble
    GetHexadecimalTile          ; 8|8/7
    ld [hl+], a                 ; 1|2   write tile to BG map; HL += 1
    ld a, b                     ; 1|1   A = ALB
    and a, $0F                  ; 2|2   A = ALB low nibble
    GetHexadecimalTile          ; 8|8/7
    ld [hl], a                  ; 1|2   write tile to BG map

;* Now we want to live update the uncovered part of the Random Value Map. At
;* first we want to know how many vales we should render. Four values determine
;* that:
;*
;*   1. Generate Address `GENERATEADDR`: contains the next value's address to
;*      be generated. Thus the last random value that is generated is located
;*      at `GENERATEADDR-1`. Runs from $C000 to $E000 during the generator
;*      process.
;*   2. Random Value Map Display Offset `DISPOFFSET`: it has the topleft value's
;*      address. As the random map has 256 values, we will only show random
;*      values from address `DISPOFFSET` to `DISPOFFSET+255` inclusive.
;*   3. Next Value To Show Address `GENERATESHOWADDR`: contains the address of
;*      the next random value to be shown by this code part. Initially set to
;*      `DISPOFFSET` and will be incremented until it ends up as
;*      `DISPOFFSET+256`.
;*   4. We define an 8 bit size upper bound of values which can get rendered in
;*      a single V-Blank period: `GENERATECDCTRUB`
;*
;* Thus, the number of values which we will render `CDCTR` is:
;* `C = MIN(GENERATECDCTRUB,MIN(DISPOFFSET+256,GENERATEADDR)-GENERATESHOWADDR)`
;* The index of the shown value is `I = GENERATESHOWADDR - DISPOFFSET`.
;* Note that if `GENERATEADDR <= DISPOFFSET` OR
;* `GENERATESHOWADDR == DISPOFFSET+256` then we are done and this two tests we
;* are going to do first.
;.update_random_map

;* Test if `GENERATEADDR <= DISPOFFSET`.
.testGAleqDO                    ; 20|25/11; max 24 cycles if not done here
    ld a, [DISPOFFSET]          ; 2|3   A = Display Offset high byte
    ld d, a                     ; 1|1   D = Display Offset high byte
    ld a, [GENERATEADDR]        ; 2|3   A = Generate Address high byte
    cp a, d                     ; 1|1   CY if A < D; NZ if A != D
    jr c, .done                 ; 2|3/2 if CY then done
    ld b, a                     ; 1|1   B = Generate Address high byte
    ld a, [GENERATEADDR+1]      ; 2|3   A = Generate Address low byte
    ld c, a                     ; 1|1   C = Generate Address low byte
    ld a, [DISPOFFSET+1]        ; 2|3   A = Display Offset low byte
    ld e, a                     ; 1|1   E = Display Offset low byte
    jr nz, .testGSAexDOplus256  ; 2|3/2 if NZ then goto next test
    cp a, c                     ; 1|1   NCY if C <= A
    jr nc, .done                ; 2|3/2 if NCY then done

;* Test if `GENERATESHOWADDR == DISPOFFSET+256`. We will do this while also
;* calculating `I`. The above equality is identical to `I` = $0100. And as we
;* might require `I` anyway if there is any tile to draw, we better calculating
;* it now. Note that `I` take 8 bits only as if it would be $0100 we break the
;* process. If we subtract the `DISPOFFSET` low byte from the `GENERATESHOWADDR`
;* low byte and got zero then we either have the `I`=0  or we are nothing to do,
;* which is decided by the comparision of the high bytes. If those are equal,
;* we are done, otherwise `GENERATESHOWADDR` high byte must be equal to
;* `DISPOFFSET` + 1 which is redundant.
.testGSAexDOplus256             ; 12|15/9; max 14 cycles if not done here
    ld a, [GENERATESHOWADDR+1]  ; 2|3   A = Show Address low byte
    ld l, a                     ; 1|1   L = Show Address low byte
    sub a, e                    ; 1|1   NZ if A != E
    ld l, a                     ; 1|1   L = I (if I != $0100)
    jr nz, .getcdctr_start      ; 2|3/2 if NZ then goto get countdown counter
    ld a, [GENERATESHOWADDR]    ; 2|3   A = Show Address high byte
    cp a, d                     ; 1|1   NZ <=> A = D + 1; Z <=> I = 0
    jr nz, .done                ; 2|3/2 if NZ then done

;* Get the countdown counter value.
.getcdctr_start                 ; 3|3   .getcdctr_start TOTAL

;* We will surely need the counter upper bound so let's load that now!
    ld a, GENERATECDCTRUB       ; 2|2   A = CDCTR upper bound
    ld h, a                     ; 1|1   H = CDCTR upper bound


;* First we calculate `MIN(DISPOFFSET+256,GENERATEADDR)`. We compare the high
;* bytes first then the low bytes.
.getcdctr1                      ; 15|14/8 (2a:14/8 cycles; 2b:10/9 cycles)
    ld a, d                     ; 1|1   A = Display Offset high byte
    inc a                       ; 1|1   A = Display Offset high byte + 1
    cp a, b                     ; 1|1   CY & NZ if A < B; result in AE;
    jr nz, ._getcdctr1cpdone    ; 2|3/2 skip comparison of low bytes if NZ.
    ld a, c                     ; 1|1   A = Generate Address low byte
                                ;       E = Display Offset low byte
    cp a, e                     ; 1|1   CY if A < E; result in BC;
                                ;       BC = GENERATEADDR
    jr c, .getcdctr2b           ; 2|3/2
    ld a, d                     ; 1|1   A = Display Offset high byte
    inc a                       ; 1|1   A = Display Offset high byte + 1
                                ;       result in AE = DISPOFFSET+256
    jr .getcdctr2a              ; 2|3
._getcdctr1cpdone
    jr nc, .getcdctr2b          ; 2|3/2 NCY & NZ if B < A; result in BC: fine
                                ;       BC = GENERATEADDR


;* Now we subtract `GENERATESHOWADDR` from the previous result which is either
;* in BC followed by a jump down or at this point of code, in AE and is
;* `DISPOFFSET+256`. We do not load `GENERATESHOWADDR` again from the RAM,
;* instead we use the following equation: `GENERATESHOWADDR = I + DISPOFFSET`.
;* Note that we do not require BC to store `GENERATEADDR` after this so we will
;* store the result in C as the counter should fit in one byte.

;* So at this point we want to know `DISPOFFSET+256-(DISPOFFSET+I)=256-I` which
;* is the two's complement of I which is the (one's complement of I) + 1. If the
;* addition results a set CY flag, we must set the result to `GENERATECDCTRUB`.
.getcdctr2a                     ; 8|9/7 (.cdctra2cwithub:9, .setcdctrub:7)
    ld a, l                     ; 1|1   A = I
    cpl                         ; 1|1   A = 255 - I (one's complement)
    add a, 1                    ; 2|2   A = CDCTR0 = 256 - I (two's complement)
                                ;       do not replace this line with `inc a`!
    jr c, .setcdctrub           ; 2|3/2 CY set -> counter = upper bound
    jr .cdctra2cwithub          ; 2|3   otherwise check agains upper bound

.getcdctr2b                     ; 10|11/10 (.setcdctrub:11,.cdctra2cwithub:10)
;* Here we want `BC-DISPOFFSET-I` into C. The result's low byte can be zero only
;* if the high byte is one (as we already jumped to .done in the other case) and
;* then we apply the upper bound.
    ld a, c                     ; 1|1   A = Generate Address low byte
    sub a, e                    ; 1|1   A = A - Display Offset low byte
    ld c, a                     ; 1|1   C = BC - DISPOFFSET low byte
    ld a, b                     ; 1|1   A = Generate Address high byte
    sbc a, d                    ; 1|1   A = A - Display Offset high byte - CY
    ld b, a                     ; 1|1   B = BC - DISPOFFSET high byte
    ld a, c                     ; 1|1   A = BC - DISPOFFSET low byte
    sub a, l                    ; 1|1   A = CDCTR low byte = CDCTR
    jr z, .setcdctrub           ; 2|3/2 otherwise apply upper bound.

.cdctra2cwithub                 ; 5|6/5
    ld c, a                     ; 1|1   C = CDCTR0
    ld a, h                     ; 1|1   A = CDCTR (upper bound)
    cp a, c                     ; 1|1   CY if upper bound < C
    jr nc, .ifromltob           ; 2|3/2 NCY -> done with counter which is in C

.setcdctrub
    ld c, h                     ; 1|1   C = CDCTR (upper bound)
.ifromltob
    ld b, l                     ; 1|1   B = I

;* >> The above part of the V-Blank handler is 152 bytes and the worst case
;* >> scenario (2a->cdctra2c) takes 158 cycles.

;* At this point we have the value index in B and the countdown counter in C.
;* However, we do not want to draw a tile upon the GENERATING window which is
;* in the $0990 -- $999B -- $9A3B -- $9A30 BG map area. For the value index, the
;* same area is $40 -- $4B -- $9B -- $90. Practically if the high nibble is
;* between $4--$9 inclusive and the low nibble <= $B then we skip the draw but
;* increment the index and decrement the counter.

.drawrandvaltileloop            ; 17|16/6
    ld a, b                     ; 1|1   A = I
    cp a, $40                   ; 2|2   CY if A < $40
    jr c, .drawrandvaltile      ; 2|3/2
    cp a, $9C                   ; 2|2   NCY if $9C <= A
    jr nc, .drawrandvaltile     ; 2|3/2
    and a, %00001100            ; 2|2
    cp a, %00001100             ; 2|2   Z if $C <= I low nibble
    jr z, .drawrandvaltile      ; 2|3/2
    jr .stepbc                  ; 2|2

.drawrandvaltile                ; 3|42
    call randmap_val_blank      ; 3|6+36 draw random value onto the Randval Map

;* Now we increment the index and decrement the counter. If the counter is
;* positive after that, then we continue drawing tiles.

.stepbc                         ; 4|5/4
    inc b                       ; 1|1   I -= 1
    dec c                       ; 1|1   CDCTR += 1; Z if we are done for this VB
    jr nz, .drawrandvaltileloop ; 2|3/2

;* >> The whole .drawrandvaltileloop above is 24 bytes and the worst case
;* >> scenario takes 63 cycles. Thus it may take `GENERATECDCTRUB * 63 - 1`
;* >> cycles total.

;* We save the final `GENERATESHOWADDR` which is `DISPOFFSET+I`. `DISPOFFSET`
;* sits in DE register pair and I in B. If `I == 0` then we are done.
.saveshowval                    ; 16|20
    ld a, b                     ; 1|1   A = I
    and a, a                    ; 1|1   ≡ cp a, 0; Z set if I = 0
    ld a, [DISPOFFSET+1]        ; 2|3   A = Display Offset low byte
    scf                         ; 1|1   initially set the carry flag
    jr z, ._genshowaddrcyset    ; 2|3/2 if Z then CY is fine
    add a, b                    ; 1|1   A = (Display Offset + I) low byte
._genshowaddrcyset
    ld [GENERATESHOWADDR+1], a  ; 2|3
    ld a, [DISPOFFSET]          ; 2|3   A = Display Offset high byte
    adc a, 0                    ; 2|2
    ld [GENERATESHOWADDR], a    ; 2|3

.done                           ; 5|16
    EndOfVblankHandler          ; 5|16

;* >> The whole generate_vblankhandler above is 197 bytes and the worst case
;* >> scenario takes `193 + 63 * GENERATECDCTRUB` number of cycles. Add the
;* >> master V-Blank handler's 32 cycle to this and the sum must be less than
;* >> the V-Blank period which is 1140 cycles.
;* >> `GENERATECDCTRUB < (1140 - 193 - 32) / 63 = 915 / 63 ≈ 14,52`
;* >> Thus we set GENERATECDCTRUB to 14.
;* >> Note that we stop writing to the VRAM earlier than the end of the handler
;* >> subroutine which may let us to draw some more tiles but we do not go that
;* >> tight; instead we prefer to get the whole routine done before the V-Blank
;* >> ends.


;* Once we are there to render a random value we also want to know whether it is
;* part of the uncovered area. For that we require the 8 bit index (`ix`) value
;* of the random value which is `GENERATESHOWADDR - DISPOFFSET - 1`. Note that
;* we consider `GENERATESHOWADDR` incremented by this time. As the $60 bytes
;* long (6 lines) GENERATING window starts from index $40 (start of the 5th
;* line), if `ix < $40` or `$A0 <= ix` then we can render the value. Otherwise
;* we should get the value of the `generate_background` which has index of
;* `ix-$40`. If that value is zero, we can also render the random value.

SECTION	"Random Number Generating Screen", ROM0

; This subroutine draws a new clear Random Number Generating Screen. The tile
; values are stored at the `generate_background` label in 96 bytes. Zero values
; represent tiles to be skipped.
generate_new:
    ld hl, generate_background  ; 3|4   tile data
    ld de, $9990                ; 3|4   destination start address (VRAM)
.repeat
    ld a, [hl+]                 ; 1|2   A = tile
    and a, a                    ; 1|1   ≡ cp a, 0
    jr z, .afterwrite           ; 2|3/2 if zero, go to next without displaying
    ld b, a                     ; 1|1   B = tile
    WaitForAnyBlank             ; 6|?
    ld a, b                     ; 1|1   A = tile
    ld [de], a                  ; 1|2   write tile to BG map
.afterwrite
    inc de                      ; 1|1   increment destination address
    ld a, l                     ; 1|1   after 96 tiles, we are done
    and a, %01111111            ; 2|2   ...
    cp a, 96                    ; 2|2   ...
    ret z                       ; 1|5/2 ___
    and a, $0F                  ; 2|2   if not at row end (every 16), go to next
    jr nz, .repeat              ; 2|3/2 ___
    Add16 de, 16                ; 8|8   at row end, adjust DE; DE += 16
    jr .repeat                  ; 2|3


; This subroutine initialize/reset the attached RNG and dumps all of the
; registers into the HRAM thereafter so that these registers can be set back
; every time the RNG is called to generate a new random number.
reset_rng:
    call rand_init              ; 3|6
    DumpAllRegisters RNGREGST   ; 28|35
    ret                         ; 1|4

generate_rand_data:

;* At first we set up and initialize the slave V-Blank handler. We copy the
;* DISPOFFSET value to GENERATESHOWADDR to indicate that no value tile has been
;* rendered yet.

    ld a, [DISPOFFSET]          ; 2|3   copy `DISPOFFSET` to `GENERATESHOWADDR`
    ld [GENERATESHOWADDR], a    ; 2|3   ...
    ld a, [DISPOFFSET+1]        ; 2|3   ...
    ld [GENERATESHOWADDR+1], a  ; 2|3   ___
    SetVblankHandler          \ ; 8|10  set generate_vblankhandler as slave
        GENERATEVBHADDR

;* Now we can fill up address range $C000 to $E000 (exclusive) with random
;* values.

    ld hl, $C000                ; 3|4   set HL to destination address
    DumpHL GENERATEADDR         ; 6|6
.repeat
    LoadAllRegisters RNGREGST   ; 27|40
    call rand                   ; 3|6   random byte -> [RNGREGST]
    DumpAllRegisters RNGREGST   ; 28|43
    LoadHL GENERATEADDR         ; 6|8
    LoadA RNGREGST              ; 2|3
    ld [hl+], a                 ; 1|2   loads value to destination address
    DumpHL GENERATEADDR         ; 6|8   this also increments the fake rDIV
.end_check                      ;       HL == $9A34; .done; .end_check_break
    ld a, h                     ; 1|1
    cp $E0                      ; 2|2
    jr nz, .repeat              ; 2|2/3
    ld a, l                     ; 1|1
    and a, a                    ; 1|1   ≡ cp a, 0
    jr nz, .repeat              ; 2|2/3
.done

;* Once we are done with generating the values we ensure that the done state and
;* the random values are shown on the screen, and then we unset the slave
;* V-Blank handler. The upper bound of the countdown counter defines how many
;* random values are shown at most in one V-Blank period. In the worst case
;* scenario none are already shown on this point so we need to wait
;* (256/counter) + 1 frames to ensure them alee shown.

    ld a, (256/GENERATECDCTRUB)+1 ; 2|2 wait one frame to show the final state
    ld c, a                     ; 1|1   ...
    WaitFrames c                ; 18|?  ___
    UnsetVblankHandler          ; 4|5
    ret                         ; 1|4


SECTION	"Random Number Generating Screen Background", ROM0, ALIGN[7]
generate_background:
    INCBIN "MODE/GENERATE.BIN"


ENDC; GENERATE_INC
