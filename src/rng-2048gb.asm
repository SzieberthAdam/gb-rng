;* RNG-2048GB

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

;* GB-RNG adaptation and comments Copyright (c) 2019 Szieberth Ádám

;* =============================================================================
;* ABSTRACT
;* =============================================================================

;* This file contains the random number generator of the game named 2048-gb. It
;* simply XOR's the one byte seed with the Divider Register value. In 2048-gb,
;* the routine is called by the Vblank handler and when new tile is needed,
;* basically after randomly timed user inputs. For games which require only a
;* single byte of randomness at a time, it might be a good choice as a RNG can
;* not get more plain than this.


;* =============================================================================
;* INCLUDES
;* =============================================================================

INCLUDE "HARDWARE.INC"
INCLUDE "RNG.INC"

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

SECTION "RNG", ROM0,ALIGN[12]

rand_init::
    ret                         ; 1|4


;* =============================================================================
;* RANDOM NUMBER GENERATOR
;* =============================================================================

;* The RNG simply XOR's the previous random value with the value of the divider
;* register.

rand::
    ld a, [rDIV]                ; 2|3   LDH
    ld b, a                     ; 1|1
    ld a, [RNGSEED]             ; 2|3   LDH
    xor a, b                    ; 1|1
    ld [RNGSEED], a             ; 2|3   LDH
    ret                         ; 1|4

                                ; 9|15  TOTAL (11|17 if RNGSEED in WRAM)

;* =============================================================================
;* REMARKS
;* =============================================================================

;* This RNG is obviously a week one. There are visible visual patterns in the
;* generated stream. For instance values at the 2nd, 6th, and 10th positions are
;* always identical.

;* However, this RNG is not designed to produce long streams. Moreover it should
;* be only used to get a single byte or less of randomness after a user input.
;* This might fit for some games. Use the least significant bits if less than a
;* byte is enough.


;* =============================================================================
;* REFERENCES
;* =============================================================================

;* [2048-GB]    Sanqui et al.: 2048-gb (GAME)
;*              https://gbhh.avivace.com/game/2048gb
