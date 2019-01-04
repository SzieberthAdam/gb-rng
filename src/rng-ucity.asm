;* RNG-UCITY
;* (Adapted) Copyright (c) 2019 Szieberth Ádám

;* Derived from the µCity 1.2 ([UCITY]) source code with the following LICENSE
;* which also applies to this file:
;* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;* Copyright (c) 2017-2018 Antonio Niño Díaz (AntonioND/SkyLyrac)
;*
;* This program is free software: you can redistribute it and/or modify
;* it under the terms of the GNU General Public License as published by
;* the Free Software Foundation, either version 3 of the License, or
;* (at your option) any later version.
;*
;* This program is distributed in the hope that it will be useful,
;* but WITHOUT ANY WARRANTY; without even the implied warranty of
;* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;* GNU General Public License for more details.
;*
;* You should have received a copy of the GNU General Public License
;* along with this program.  If not, see <http://www.gnu.org/licenses/>.
;*
;* Contact: antonio_nd@outlook.com
;* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


;* =============================================================================
;* ABSTRACT
;* =============================================================================

;* This file contains the random number generator of the game named µCity.


;* =============================================================================
;* INCLUDES
;* =============================================================================

INCLUDE "HARDWARE.INC"
INCLUDE "GBRNG.INC"


;* =============================================================================
;* INITIALIZATION
;* =============================================================================

;* The game seeded the RNG with the initial random values of the RAM. All values
;* of the RAM are added which yields the initial 8 bit RNG seed which is then
;* stored in the WRAM0:
;*
;*     ld      hl,_RAM
;*     ld      bc,$2000
;*     ld      e,$00
;* .random_seed_loop:
;*     ld      a,e
;*     add     a,[hl]
;*     ld      e,a
;*     inc     hl
;*     dec     bc
;*     ld      a,b
;*     or      a,c
;*     jr      nz,.random_seed_loop
;*     ld      a,e
;*     push    af
;*       ...
;*     pop     af
;*     call    SetRandomSeed
;*       ...
;*     SECTION "RandomPtr",WRAM0
;* random_ptr: DS 1
;*       ...
;* SetRandomSeed::
;*     ld      [random_ptr],a
;*     ret


;* µCity uses a fixed random lookup table:

SECTION "RandomLUT",ROM0,ALIGN[8]

_Random:
; LowN  0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F / HighN
    DB $29,$23,$be,$84,$e1,$6c,$d6,$ae,$52,$90,$49,$f1,$f1,$bb,$e9,$eb; 0
    DB $b3,$a6,$db,$3c,$87,$0c,$3e,$99,$24,$5e,$0d,$1c,$06,$b7,$47,$de; 1
    DB $b3,$12,$4d,$c8,$43,$bb,$8b,$a6,$1f,$03,$5a,$7d,$09,$38,$25,$1f; 2
    DB $5d,$d4,$cb,$fc,$96,$f5,$45,$3b,$13,$0d,$89,$0a,$1c,$db,$ae,$32; 3
    DB $20,$9a,$50,$ee,$40,$78,$36,$fd,$12,$49,$32,$f6,$9e,$7d,$49,$dc; 4
    DB $ad,$4f,$14,$f2,$44,$40,$66,$d0,$6b,$c4,$30,$b7,$32,$3b,$a1,$22; 5
    DB $f6,$22,$91,$9d,$e1,$8b,$1f,$da,$b0,$ca,$99,$02,$b9,$72,$9d,$49; 6
    DB $2c,$80,$7e,$c5,$99,$d5,$e9,$80,$b2,$ea,$c9,$cc,$53,$bf,$67,$d6; 7
    DB $bf,$14,$d6,$7e,$2d,$dc,$8e,$66,$83,$ef,$57,$49,$61,$ff,$69,$8f; 8
    DB $61,$cd,$d1,$1e,$9d,$9c,$16,$72,$72,$e6,$1d,$f0,$84,$4f,$4a,$77; 9
    DB $02,$d7,$e8,$39,$2c,$53,$cb,$c9,$12,$1e,$33,$74,$9e,$0c,$f4,$d5; A
    DB $d4,$9f,$d4,$a4,$59,$7e,$35,$cf,$32,$22,$f4,$cc,$cf,$d3,$90,$2d; B
    DB $48,$d3,$8f,$75,$e6,$d9,$1d,$2a,$e5,$c0,$f7,$2b,$78,$81,$87,$44; C
    DB $0e,$5f,$50,$00,$d4,$61,$8d,$be,$7b,$05,$15,$07,$3b,$33,$82,$1f; D
    DB $18,$70,$92,$da,$64,$54,$ce,$b1,$85,$3e,$69,$15,$f8,$46,$6a,$04; E
    DB $96,$73,$0e,$d9,$16,$2f,$67,$68,$d4,$f7,$4a,$4a,$d0,$57,$68,$76; F


SECTION "RNG", ROM0

rand_init::
    ret                         ; 1|4


;* =============================================================================
;* RANDOM NUMBER GENERATOR
;* =============================================================================

;* The seed value determines which value of the table below gets XOR'ed with the
;* divider register value. Thereafter the next value of the table is added to
;* the result and this new value is the random value produced and also the new
;* seed value.

rand::
    ld hl, GBRNG_RAMSEED        ; 3|4
    ld l, [hl]                  ; 1|2   low byte of the random table address
    ld h, _Random >> 8          ; 2|2   high byte of the random table address

    ld a, [rDIV]                ; 2|2   LDH
    xor a, [hl]                 ; 1|2   XOR with the table value

    inc l                       ; 1|1   sets the address to the next table value
    add a, [hl]                 ; 1|2   ADD the next table value

    ld hl, GBRNG_RAMSEED        ; 3|4
    ld [hl], a                  ; 1|2   set the random value the new seed value

    ret                         ; 1|4

                                ; 16|25 TOTAL

;* =============================================================================
;* REMARKS
;* =============================================================================

;* I am suspicious that this RNG gets more and more deterministic with each
;* yielded byte. If you look at the lower right tile of the screen and reset
;* the BGB emulator several times, you should notice that particular tile being
;* one of the tree distinct tiles it could be.

;* I can not exclude the possibility that I made a mistake with the migration of
;* AntoniND's code despite it is identical except for the initial seed
;* calculation. However if I did it right then this might be a bug in it.

;* Note that µCity do not update the seed with user imputs.

;* Not recommended.


;* =============================================================================
;* REFERENCES
;* =============================================================================

;* [UCITY]      Antonio Niño Díaz: µCity (GAME)
;*              https://github.com/AntonioND/ucity
