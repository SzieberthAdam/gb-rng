;* GBRNG.INC
;* Copyright (c) 2019 Szieberth Ádám
;* 0BSD License (see LICENSE file for more info)

;* =============================================================================
;* ABSTRACT
;* =============================================================================

;* This include files contains the BG-RNG constants and some of the most
;* essential macros.


;* =============================================================================
;* THE CODE
;* =============================================================================

IF !DEF(GBRNG_INC)
GBRNG_INC = 1


;* =============================================================================
;* NAMED MEMORY ADDRESSES
;* =============================================================================

;* GB-RNG has a master V-Blank handler which does a `jp hl` to pass the handling
;* to the slave handler which is set by the actual mode (room) of the
;* application. However as RGBASM is a one pass assembler, we must set static
;* addresses for the slave V-Blank handlers. As roman V is 5, we reserve the
;* $5000+ space for them.

GENERATEVBHADDR EQU $5000
RANDMAPVBHADDR EQU $5100
VALMAPVBHADDR EQU $5200

;* As GB-RNG uses the whole WRAM to store the generated random values, all we
;* remained is the HRAM space. Note that there is an RNG.INC too which defines
;* the address of the seed for any attached RNG. This makes most of the RNGs
;* nicely portable as they include the RNG.INC and/or HARDWARE.INC only. RNG.INC
;* set the seed address to $FF80 and as GB-RNG provides an 8 bytes seed which is
;* then copied to make the user able to restore it anytime, the seed area which
;* is not controlled by this file is 16 bytes long. This makes $FF90 our first
;* available address.

;* GB-RNG provides the attached RNG a whoile set of own registers. The state of
;* the registers are dumped after the `rand` subroutine is called and loaded
;* back immediately before the next one. We store those register values in an
;* eight bytes space at $FF90.
RNGREGST EQU $FF90              ;       8 bytes for AF BC DE HL

;* GB-RNG's main loop jumps to the various rooms according to their mode value.
;* We store the application mode in one byte.
MODE EQU $FFA0

;* As it was mentioned above, the master V-Blank handler jumps to the given
;* slave. The address of the slave V-Blank handler is stored in 2 bytes starting
;* from the address below. Order is HL.
SLAVEVBHADDR EQU $FFA1

;* Key events are handled specifically to the game mode by the slave V-Blank
;* handlers but some key related data is common. KEYNEW stores the current
;* pressed keys while KEYOLD stores the pressed keys of the previous frame.
;* KEYHOLD is a 8 bit long frame counter for the relevant key which is hold by
;* the user.
KEYNEW EQU $FFA3
KEYOLD EQU $FFA4
KEYHOLD EQU $FFA5

;* The most important mode of the GB-RNG application is the Random Area Map, a
;* 16x16 size viewport to the generated random values sitting in $C000-$DFFF.
;* We have to store the viewport's starting address to be able to move it. It
;* requires 16 bits.
DISPOFFSET EQU $FFAA

;* Another important mode is the Generate Screen which is shown during the RNG
;* process. It is a small window in the center of the screen with a progress bar
;* and a label with the address of the random number which is being generated.
;* The Generate Screen quazi live updates the random values of the background
;* during the process. To be able to do that, its slave V-Blank handler must
;* store the last value's 16 bit address which has been updated in the
;* background, the Random Map viewport.
GENERATESHOWADDR EQU $FFAC

;* The generator have to store the 16 bit address of the next random value to be
;* generated. The address can be $C000-$E000. If it is $E000 then the generator
;* stops.
GENERATEADDR EQU $FFAE


;* =============================================================================
;* CONSTANT VALUES
;* =============================================================================

;* We set the horizontal scroll register to 12 tiles right and the vertical
;* scroll register to 6 tiles down so that $98CC will be the address of the top
;* left tile of the static GB-RNG screen. $9AFF will be the bottom right tile's
;* address. This adjustment has the advantage of placing the random map area's
;* top left tile to $9910. That results the lowest nibble of the VRAM address
;* to match with the low nibble of the random value's address. The high nibble
;* could be more optimized but only if we would split the screen which would
;* result more trouble with other rendering tasks.
DISPTSCX EQU 12
DISPTSCY EQU 6

;* Below is the upper bound of the Generate Screen's countdown counter. It
;* should be set to support the visual experience of live drawing of the
;* generated random values and also it must keep the Generate Screen's V-Blank
;* handler within the V-Blank period. It turned out that the second criteria
;* maximizes `GENERATECDCTRUB` in 14 which is thankfully also support our first
;* criteria.
GENERATECDCTRUB EQU 14


;* =============================================================================
;* MACROS
;* =============================================================================

; This macro unsets the mode specific V-Blank handler. A 16bit address should be
; given as an argument. As the master V-Blank handler passes the handling when
; the high byte is not $E0, we write the low byte first as we do not want to be
; redirected to a partly set memory address.
; @destroy A
SetVblankHandler: MACRO
    ld a, \1 & $FF              ; 2|2   A = low byte
    ld [SLAVEVBHADDR+1], a      ; 2|3
    ld a, \1 >> 8               ; 2|2   A = high byte
    ld [SLAVEVBHADDR], a        ; 2|3
ENDM                            ; 8|10 TOTAL


; This macro unsets the mode specific V-Blank handler.
; @destroy A
UnsetVblankHandler: MACRO
    ld a, $E0                   ; 2|2
    ld [SLAVEVBHADDR], a        ; 2|3
ENDM                            ; 4|5 TOTAL


; IMPORTANT!
; This macro must be called at the end of every slave V-Blank handler!
; It restores the registers and returns from the interrupt.
EndOfVblankHandler: MACRO
    pop hl                      ; 1|3
    pop de                      ; 1|3
    pop bc                      ; 1|3
    pop af                      ; 1|3
    reti                        ; 1|4
ENDM                            ; 5|16 TOTAL


ENDC ;GBRNG_INC
