;* RNG-2048GB
;* Copyright (c) 2018 Szieberth Ádám

;* Derived from the 2048-gb ([2048-GB]) source code with the following LICENSE
;* which also applies to this file if anyone thinks that a license can be
;* applied to this tiny code:
;* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;* Copyright (c) 2014 "Sanqui"
;*
;* This software is provided 'as-is', without any express or implied
;* warranty. In no event will the authors be held liable for any damages
;* arising from the use of this software.
;*
;* Permission is granted to anyone to use this software for any purpose,
;* including commercial applications, and to alter it and redistribute it
;* freely, subject to the following restrictions:
;*
;*    1. The origin of this software must not be misrepresented; you must not
;*    claim that you wrote the original software. If you use this software
;*    in a product, an acknowledgment in the product documentation would be
;*    appreciated but is not required.
;*
;*    2. Altered source versions must be plainly marked as such, and must not be
;*    misrepresented as being the original software.
;*
;*    3. This notice may not be removed or altered from any source
;*    distribution.
;* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


;* =============================================================================
;* ABSTRACT
;* =============================================================================

;* This file contains the random number generator of the game named 2048-gb. The
;* game has a very simple RNG which might be enough for its purpose.


;* =============================================================================
;* INCLUDES
;* =============================================================================

;* HARDWARE.INC contains the 'Hardware Defines' for our program. This has
;* address location labels for all of the GameBoy Hardware I/O registers. We can
;* 'insert' this file into the present ASM file by using the assembler INCLUDE
;* command:
INCLUDE "HARDWARE.INC"


;* =============================================================================
;* INITIALIZATION
;* =============================================================================

;* The game seeded the RNG with the initial random values of the RAM. It set HL
;* to $C000 then set L to [$C000] (RNG seed was stored in H_RNG1/$FFF1):
;*
;*     ld hl, $C000
;*     ld l, [hl]
;*     ld a, [hl]
;*     push af
;*       ...
;*     pop af
;*     ld [H_RNG1], a

H_RNG1 EQU $FFF1                ;       we retain the address used by the game

SECTION "RNG", ROM0

;* We have a nice random seed value in $DF00 so we copy that to H_RNG1:

rand_init::
    ld a, [$DF00]               ; 3|4
    ld [H_RNG1], a              ; 3|4
    ret                         ; 1|4


;* =============================================================================
;* RANDOM NUMBER GENERATOR
;* =============================================================================

;* The RNG simply XOR's the previous random value with the value of the divider
;* register.

rand::
    ld a, [rDIV]
    ld b, a
    ld a, [H_RNG1]
    xor b
    ld [H_RNG1], a
    ret


;* =============================================================================
;* REFERENCES
;* =============================================================================

;* [2048-GB]    Sanqui et al.: 2048-gb (GAME)
;*              https://gbhh.avivace.com/game/2048gb
