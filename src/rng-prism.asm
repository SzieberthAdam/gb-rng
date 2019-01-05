;* RNG-PRISM

;* Original code is licensed as public domain by Aaaaaa123456789/ax6 which also
;* applies to this file:
;* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;* "This code was originally found in Prism. Originally written on 2016-12-18.
;* updated on 2018-08-18 with a bug fix. Since I never bothered with a license
;* header, I'm hereby placing it in the public domain as of 2019-01-04."
;* [PB.PRISM]
;* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

;* GB-RNG adaptation and comments Copyright (c) 2019 Szieberth Ádám


;* =============================================================================
;* ABSTRACT
;* =============================================================================

;* ATTENTION! As GB-RNG project is on hiatus, this code is copypasted here
;* unprocessed for completeness.


;* =============================================================================
;* CHANGES
;* =============================================================================


;* =============================================================================
;* INCLUDES
;* =============================================================================

INCLUDE "GBRNG.INC"


;* =============================================================================
;* INITIALIZATION
;* =============================================================================

SECTION "RNG", ROM0

rand_init::
    ld hl, GBRNG_RAMSEED        ; 3|3
    ret                         ; 1|4

;* =============================================================================
;* RANDOM NUMBER GENERATOR
;* =============================================================================

rand::
	; in: hl: pointer to 8-byte RNG state
	; out: a: random value; other registers preserved
	push bc
	push de
	push hl
	call .advance_left_register
	call .advance_right_register
	inc hl
	inc hl
	inc hl
	call .advance_selector_register
	pop hl
	push hl
	rlca
	rlca
	ld c, a
	and 3
	ld e, a
	ld d, 0
	add hl, de
	ld b, [hl]
	pop hl
	push hl
	ld e, 5
	add hl, de
	ld a, c
	ld c, [hl]
	rlca
	rlca
	and 3
	call .combine_register_values
	pop hl
	pop de
	pop bc
	ret

.advance_left_register
	; in: hl: pointer to left register
	; out: hl: pointer to RIGHT register
	ld a, [hli]
	ld e, a
	ld a, [hli]
	ld d, a
	ld a, [hli]
	ld c, a
	ld a, [hld]
	ld b, a
	or c
	or d
	or e
	call z, .reseed_left_register
	ld a, e
	xor d
	ld e, a
	ld a, d
	xor c
	ld d, a
	ld a, c
	xor b
	ld c, a
	ld a, c
	ld [hld], a
	ld a, d
	ld [hld], a
	ld [hl], e
	sla e
	rl d
	rl c
	inc hl
	ld a, [hl]
	xor e
	ld [hli], a
	ld a, [hl]
	xor d
	ld [hli], a
	ld a, [hl]
	xor c
	ld [hld], a
	ld b, a
	ld c, [hl]
	sla c
	rl b
	sbc a
	and 1
	dec hl
	xor [hl]
	ld [hld], a
	ld a, [hl]
	xor b
	ld [hli], a
	inc hl
	inc hl
	inc hl
	ret

.reseed_left_register
	; in: hl: pointer to left register + 2
	; out: hl preserved; bcde new seed
	ld de, 5
	push hl
	add hl, de
	call .advance_selector_register
	ld b, a
	call .advance_selector_register
	ld c, a
	call .advance_selector_register
	ld d, a
	call .advance_selector_register
	ld e, a
	pop hl
	inc hl
	ld a, b
	ld [hld], a ;only b needs to be written back, since the rest will be handled by the main function
	ret

.advance_right_register
	; in: hl: pointer to right register
	; out: hl preserved
	ld a, [hli]
	cp 210
	jr c, .right_carry_OK
	sub 210
.right_carry_OK
	ld d, a
	ld a, [hli]
	ld e, a
	ld c, [hl]
	or c
	or d
	jr z, .right_register_needs_reseed
	ld a, c
	and e
	inc a
	jr nz, .right_register_OK
	ld a, d
	cp 209
	jr nz, .right_register_OK
.right_register_needs_reseed
	call .reseed_right_register
.right_register_OK
	ld a, e
	ld [hld], a
	push hl
	ld b, 0
	ld h, b
	ld l, d
	ld a, 210

.loop
	add hl, bc
	dec a
	jr nz, .loop

	ld a, l
	ld b, h
	pop hl
	ld [hld], a
	ld [hl], b
	ret

.reseed_right_register
	; in: hl: pointer to right register + 2
	; out: hl preserved, cde new seed
	inc hl
	call .advance_selector_register
	ld c, a
	call .advance_selector_register
	ld d, a
	call .advance_selector_register
	ld e, a
	dec hl
	ret

.advance_selector_register
	; in: hl: pointer to selector register
	; out: all registers but a preserved; a = new selector
	push bc
	ld a, [hl]
	ld b, 0
	rra
	rr b
	rra
	rr b
	ld a, [hl]
	swap a
	rrca
	and $f8
	add a, b
	add a, [hl]
	add a, 29
	ld [hl], a
	pop bc
	ret

.combine_register_values
	and a
	jr z, .add_registers
	dec a
	jr z, .xor_registers
	dec a
	jr z, .subtract_registers
	ld a, c
	sub b
	ret
.subtract_registers
	ld a, b
	sub c
	ret
.add_registers
	ld a, b
	add a, c
	ret
.xor_registers
	ld a, b
	xor c
	ret


;* =============================================================================
;* REMARKS
;* =============================================================================


;* =============================================================================
;* REFERENCES
;* =============================================================================

;* [PB.PRISM]   Aaaaaa123456789/ax6: StableRandom's original gbz80 code
;*              https://pastebin.com/KF8uk9B1
