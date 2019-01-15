;* REGISTER.INC
;* Copyright (c) 2019 Szieberth Ádám
;* 0BSD License (see LICENSE file for more info)

;* =============================================================================
;* ABSTRACT
;* =============================================================================

;* This file contains macros to dump and load registers to and from given RAM
;* locations.


;* =============================================================================
;* THE CODE
;* =============================================================================

IF !DEF(MACRO_REGISTER_INC)
MACRO_REGISTER_INC SET 1

;* DumpRegisters take eight optional a16 arguments to dump A, F, B, C, D, E, H,
;* and L register values respectively. To skip dumping a register, pass 0 as its
;* argument.

;* Destroys A if does not dump F and dumps any of B, C, D, E, H, and L.
;* Destroys HL if dumps F.
;* You can call LoadRegisters to restore the destroyed registers after the dump.

;*                Size and 1 MHz Cycles chart (H: HRAM; W: WRAM)
;*
;*               | A | F | B | C | D | E | H | L | Size | Cycles |
;*               |---|---|---|---|---|---|---|---|------|--------|
;*               | H | H | H | H | H | H | H | H |   28 |     43 |
;*               | W | W | W | W | W | W | W | W |   36 |     51 |
;*               |   |   | H | H | H | H | H | H |   18 |     24 |
;*               |   |   | W | W | W | W | W | W |   24 |     30 |
;*               | H | H |   |   | H | H | H | H |   22 |     35 |
;*               | W | W |   |   | W | W | W | W |   28 |     41 |
;*               | H | H | H | H |   |   | H | H |   22 |     35 |
;*               | W | W | W | W |   |   | W | W |   28 |     41 |
;*               | H | H | H | H | H | H |   |   |   22 |     35 |
;*               | W | W | W | W | W | W |   |   |   28 |     41 |
;*               |   |   |   |   | H | H | H | H |   12 |     16 |
;*               |   |   |   |   | W | W | W | W |   16 |     20 |
;*               |   |   | H | H |   |   | H | H |   12 |     16 |
;*               |   |   | W | W |   |   | W | W |   16 |     20 |
;*               |   |   | H | H | H | H |   |   |   12 |     16 |
;*               |   |   | W | W | W | W |   |   |   16 |     20 |
;*               | H | H |   |   |   |   | H | H |   16 |     27 |
;*               | W | W |   |   |   |   | W | W |   20 |     31 |
;*               | H | H |   |   | H | H |   |   |   16 |     27 |
;*               | W | W |   |   | W | W |   |   |   20 |     31 |
;*               | H | H | H | H |   |   |   |   |   16 |     27 |
;*               | W | W | W | W |   |   |   |   |   20 |     31 |
;*               |   |   |   |   |   |   | H | H |    6 |      8 |
;*               |   |   |   |   |   |   | W | W |    8 |     10 |
;*               |   |   |   |   | H | H |   |   |    6 |      8 |
;*               |   |   |   |   | W | W |   |   |    8 |     10 |
;*               |   |   | H | H |   |   |   |   |    6 |      8 |
;*               |   |   | W | W |   |   |   |   |    8 |     10 |
;*               | H | H |   |   |   |   |   |   |   10 |     19 |
;*               | W | W |   |   |   |   |   |   |   12 |     21 |
;*               | H |   |   |   |   |   |   |   |    2 |      3 |
;*               | W |   |   |   |   |   |   |   |    3 |      4 |
;*               |   | H |   |   |   |   |   |   |    8 |     16 |
;*               |   | W |   |   |   |   |   |   |    9 |     17 |
;*               |   |   | H |   |   |   |   |   |    3 |      4 |
;*               |   |   | W |   |   |   |   |   |    4 |      5 |
;*               |   |   |   | H |   |   |   |   |    3 |      4 |
;*               |   |   |   | W |   |   |   |   |    4 |      5 |
;*               |   |   |   |   | H |   |   |   |    3 |      4 |
;*               |   |   |   |   | W |   |   |   |    4 |      5 |
;*               |   |   |   |   |   | H |   |   |    3 |      4 |
;*               |   |   |   |   |   | W |   |   |    4 |      5 |
;*               |   |   |   |   |   |   | H |   |    3 |      4 |
;*               |   |   |   |   |   |   | W |   |    4 |      5 |
;*               |   |   |   |   |   |   |   | H |    3 |      4 |
;*               |   |   |   |   |   |   |   | W |    4 |      5 |


DumpRegisters: MACRO

    IF \2 != 0                  ;       F Register
    push af                     ; 1|4
    ELIF \1 != 0                ;       A Register AND NOT F Register
    ld [\1], a                  ; 2/3|3/4
    ENDC                        ;       now we are free to use the AF register

    IF \7 != 0                  ;       H Register
    ld a, h                     ; 1|1
    ld [\7], a                  ; 2/3|3/4
    ENDC

    IF \8 != 0                  ;       L Register
    ld a, l                     ; 1|1
    ld [\8], a                  ; 2/3|3/4
    ENDC                        ;       now we are free to use the HL register

    IF \3 != 0                  ;       B Register
    ld a, b                     ; 1|1
    ld [\3], a                  ; 2/3|3/4
    ENDC

    IF \4 != 0                  ;       C Register
    ld a, c                     ; 1|1
    ld [\4], a                  ; 2/3|3/4
    ENDC

    IF \5 != 0                  ;       D Register
    ld a, d                     ; 1|1
    ld [\5], a                  ; 2/3|3/4
    ENDC

    IF \6 != 0                  ;       E Register
    ld a, e                     ; 1|1
    ld [\6], a                  ; 2/3|3/4
    ENDC

    IF \2 != 0                  ;       F Register
    ld hl, sp+0                 ; 2|3
    ld a, [hl+]                 ; 1|2
    ld [\2], a                  ; 2/3|3/4
    IF \1 != 0                  ;       F Register AND A Register
    ld a, [hl+]                 ; 1|2
    ld [\1], a                  ; 2/3|3/4
    ELSE                        ;       F Register AND NOT A Register
    inc hl                      ; 1|2
    ENDC                        ;       F Register
    ld sp, hl                   ; 1|2
    ENDC

ENDM



;* LoadRegisters take eight optional a16 arguments to load A, F, B, C, D, E, H,
;* and L register values respectively. To skip loading a register, pass 0 as its
;* argument.

;* Apart from overwriting those registers with nonzero arguments, it destroys
;* the following registers despite the respectible zero argument:
;* Destroys A.
;* Destroys HL if loads F.
;* Naturally these registers will contain the saved values if loaded.

;*                Size and 1 MHz Cycles chart (H: HRAM; W: WRAM)
;*
;*               | A | F | B | C | D | E | H | L | Size | Cycles |
;*               |---|---|---|---|---|---|---|---|------|--------|
;*               | H | H | H | H | H | H | H | H |   27 |     40 |
;*               | W | W | W | W | W | W | W | W |   35 |     48 |
;*               |   |   | H | H | H | H | H | H |   18 |     24 |
;*               |   |   | W | W | W | W | W | W |   24 |     30 |
;*               | H | H |   |   | H | H | H | H |   21 |     32 |
;*               | W | W |   |   | W | W | W | W |   27 |     38 |
;*               | H | H | H | H |   |   | H | H |   21 |     32 |
;*               | W | W | W | W |   |   | W | W |   27 |     38 |
;*               | H | H | H | H | H | H |   |   |   21 |     32 |
;*               | W | W | W | W | W | W |   |   |   27 |     38 |
;*               |   |   |   |   | H | H | H | H |   12 |     16 |
;*               |   |   |   |   | W | W | W | W |   16 |     20 |
;*               |   |   | H | H |   |   | H | H |   12 |     16 |
;*               |   |   | W | W |   |   | W | W |   16 |     20 |
;*               |   |   | H | H | H | H |   |   |   12 |     16 |
;*               |   |   | W | W | W | W |   |   |   16 |     20 |
;*               | H | H |   |   |   |   | H | H |   15 |     24 |
;*               | W | W |   |   |   |   | W | W |   19 |     28 |
;*               | H | H |   |   | H | H |   |   |   15 |     24 |
;*               | W | W |   |   | W | W |   |   |   19 |     28 |
;*               | H | H | H | H |   |   |   |   |   15 |     24 |
;*               | W | W | W | W |   |   |   |   |   19 |     28 |
;*               |   |   |   |   |   |   | H | H |    6 |      8 |
;*               |   |   |   |   |   |   | W | W |    8 |     10 |
;*               |   |   |   |   | H | H |   |   |    6 |      8 |
;*               |   |   |   |   | W | W |   |   |    8 |     10 |
;*               |   |   | H | H |   |   |   |   |    6 |      8 |
;*               |   |   | W | W |   |   |   |   |    8 |     10 |
;*               | H | H |   |   |   |   |   |   |    9 |     16 |
;*               | W | W |   |   |   |   |   |   |   11 |     18 |
;*               | H |   |   |   |   |   |   |   |    2 |      3 |
;*               | W |   |   |   |   |   |   |   |    3 |      4 |
;*               |   | H |   |   |   |   |   |   |    7 |     13 |
;*               |   | W |   |   |   |   |   |   |    8 |     14 |
;*               |   |   | H |   |   |   |   |   |    3 |      4 |
;*               |   |   | W |   |   |   |   |   |    4 |      5 |
;*               |   |   |   | H |   |   |   |   |    3 |      4 |
;*               |   |   |   | W |   |   |   |   |    4 |      5 |
;*               |   |   |   |   | H |   |   |   |    3 |      4 |
;*               |   |   |   |   | W |   |   |   |    4 |      5 |
;*               |   |   |   |   |   | H |   |   |    3 |      4 |
;*               |   |   |   |   |   | W |   |   |    4 |      5 |
;*               |   |   |   |   |   |   | H |   |    3 |      4 |
;*               |   |   |   |   |   |   | W |   |    4 |      5 |
;*               |   |   |   |   |   |   |   | H |    3 |      4 |
;*               |   |   |   |   |   |   |   | W |    4 |      5 |


LoadRegisters: MACRO

    IF \2 != 0                  ;       F Register
    ld hl, sp-2                 ; 2|3
    ld a, [\2]                  ; 2/3|3/4
    ld [hl], a                  ; 1|2
    ld sp, hl                   ; 1|2
    pop af                      ; 1|3
    ENDC

    IF \4 != 0                  ;       C Register
    ld a, [\4]                  ; 2/3|3/4
    ld c, a                     ; 1|1
    ENDC

    IF \3 != 0                  ;       B Register
    ld a, [\3]                  ; 2/3|3/4
    ld b, a                     ; 1|1
    ENDC

    IF \6 != 0                  ;       E Register
    ld a, [\6]                  ; 2/3|3/4
    ld e, a                     ; 1|1
    ENDC

    IF \5 != 0                  ;       D Register
    ld a, [\5]                  ; 2/3|3/4
    ld d, a                     ; 1|1
    ENDC

    IF \8 != 0                  ;       L Register
    ld a, [\8]                  ; 2/3|3/4
    ld l, a                     ; 1|1
    ENDC

    IF \7 != 0                  ;       H Register
    ld a, [\7]                  ; 2/3|3/4
    ld h, a                     ; 1|1
    ENDC

    IF \1 != 0                  ;       A Register
    ld a, [\1]                  ; 2/3|3/4
    ENDC
ENDM


;* =============================================================================
;* SHORTCUTS
;* =============================================================================


;* DumpAllRegisters is a one argument shortcut of DumpRegisters which requires
;* a 16 bit address as the argument and dumps all registers in 8 bytes starting
;* from the given address in A, F, B, C, D, E, H, L order.

;* Destroys HL.

;* Takes 28 bytes and 43 cycles if given address is in HRAM.
;* Takes 36 bytes and 51 cycles if given address is in WRAM.

DumpAllRegisters: MACRO
    DumpRegisters             \
        \1,\1+1,\1+2,\1+3,    \
        \1+4,\1+5,\1+6,\1+7
ENDM


;* LoadAllRegisters is a one argument shortcut of LoadRegisters which requires
;* a 16 bit address as the argument and loads all registers from 8 bytes
;* starting with the given address in A, F, B, C, D, E, H, L order.

;* Takes 27 bytes and 40 cycles if given address is in HRAM.
;* Takes 35 bytes and 48 cycles if given address is in WRAM.

LoadAllRegisters: MACRO
    LoadRegisters             \
        \1,\1+1,\1+2,\1+3,    \
        \1+4,\1+5,\1+6,\1+7
ENDM


;* DumpAll16bitRegisters is a one argument shortcut of DumpRegisters which
;* requires a 16 bit address as the argument and dumps all 16 bit registers in
;* 6 bytes starting from the given address in B, C, D, E, H, L order.

;* Destroys A.

;* Takes 18 bytes and 24 cycles if given address is in HRAM.
;* Takes 24 bytes and 30 cycles if given address is in WRAM.

DumpAll16bitRegisters: MACRO
    DumpRegisters             \
        0,0,\1,\1+1,\1+2,     \
        \1+3,\1+4,\1+5
ENDM


;* LoadAll16bitRegisters is a one argument shortcut of LoadRegisters which
;* requires a 16 bit address as the argument and loads all 16 bit registers from
;* 6 bytes starting with the given address in B, C, D, E, H, L order.

;* Destroys A.

;* Takes 18 bytes and 24 cycles if given address is in HRAM.
;* Takes 24 bytes and 30 cycles if given address is in WRAM.

LoadAll16bitRegisters: MACRO
    LoadRegisters             \
        0,0,\1,\1+1,\1+2,     \
        \1+3,\1+4,\1+5
ENDM


;* DumpBC is a one argument shortcut of DumpRegisters which requires a 16 bit
;* address as the argument and dumps the BC register in 2 bytes starting from
;* the given address.

;* Destroys A.

;* Takes 6 bytes and 8 cycles if given address is in HRAM.
;* Takes 8 bytes and 10 cycles if given address is in WRAM.

DumpBC: MACRO
    DumpRegisters             \
        0,0,\1,\1+1,0,0,0,0
ENDM


;* LoadBC is a one argument shortcut of LoadRegisters which requires a 16 bit
;* address as the argument and loads the BC register from 2 bytes starting with
;* the given address.

;* @return A = B

;* Takes 6 bytes and 8 cycles if given address is in HRAM.
;* Takes 8 bytes and 10 cycles if given address is in WRAM.

LoadBC: MACRO
    LoadRegisters             \
        0,0,\1,\1+1,0,0,0,0
ENDM


;* DumpDE is a one argument shortcut of DumpRegisters which requires a 16 bit
;* address as the argument and dumps the DE register in 2 bytes starting from
;* the given address.

;* Destroys A.

;* Takes 6 bytes and 8 cycles if given address is in HRAM.
;* Takes 8 bytes and 10 cycles if given address is in WRAM.

DumpDE: MACRO
    DumpRegisters             \
        0,0,0,0,\1,\1+1,0,0
ENDM


;* LoadDE is a one argument shortcut of LoadRegisters which requires a 16 bit
;* address as the argument and loads the DE register from 2 bytes starting with
;* the given address.

;* @return A = D

;* Takes 6 bytes and 8 cycles if given address is in HRAM.
;* Takes 8 bytes and 10 cycles if given address is in WRAM.

LoadDE: MACRO
    LoadRegisters             \
        0,0,0,0,\1,\1+1,0,0
ENDM


;* DumpHL is a one argument shortcut of DumpRegisters which requires a 16 bit
;* address as the argument and dumps the HL register in 2 bytes starting from
;* the given address.

;* Destroys A.

;* Takes 6 bytes and 8 cycles if given address is in HRAM.
;* Takes 8 bytes and 10 cycles if given address is in WRAM.

DumpHL: MACRO
    DumpRegisters             \
        0,0,0,0,0,0,\1,\1+1
ENDM


;* LoadHL is a one argument shortcut of LoadRegisters which requires a 16 bit
;* address as the argument and loads the HL register from 2 bytes starting with
;* the given address.

;* @return A = H

;* Takes 6 bytes and 8 cycles if given address is in HRAM.
;* Takes 8 bytes and 10 cycles if given address is in WRAM.

LoadHL: MACRO
    LoadRegisters             \
        0,0,0,0,0,0,\1,\1+1
ENDM


;* DumpA is a one argument shortcut of DumpRegisters which requires a 16 bit
;* address as the argument and dumps the A register to the given RAM address.

;* Takes 2 bytes and 3 cycles if given address is in HRAM.
;* Takes 3 bytes and 4 cycles if given address is in WRAM.

DumpA: MACRO
    DumpRegisters             \
        \1,0,0,0,0,0,0,0
ENDM


;* LoadA is a one argument shortcut of LoadRegisters which requires a 16 bit
;* address as the argument and loads the A register from the given RAM address.

;* Takes 2 bytes and 3 cycles if given address is in HRAM.
;* Takes 3 bytes and 4 cycles if given address is in WRAM.

LoadA: MACRO
    LoadRegisters             \
        \1,0,0,0,0,0,0,0
ENDM


;* DumpB is a one argument shortcut of DumpRegisters which requires a 16 bit
;* address as the argument and dumps the B register to the given RAM address.

;* Destroys A.

;* Takes 3 bytes and 4 cycles if given address is in HRAM.
;* Takes 4 bytes and 5 cycles if given address is in WRAM.

DumpB: MACRO
    DumpRegisters             \
        0,0,\1,0,0,0,0,0
ENDM


;* LoadB is a one argument shortcut of LoadRegisters which requires a 16 bit
;* address as the argument and loads the B register from the given RAM address.

;* @return A = B

;* Takes 3 bytes and 4 cycles if given address is in HRAM.
;* Takes 4 bytes and 5 cycles if given address is in WRAM.

LoadB: MACRO
    LoadRegisters             \
        0,0,\1,0,0,0,0,0
ENDM


;* DumpC is a one argument shortcut of DumpRegisters which requires a 16 bit
;* address as the argument and dumps the C register to the given RAM address.

;* Destroys A.

;* Takes 3 bytes and 4 cycles if given address is in HRAM.
;* Takes 4 bytes and 5 cycles if given address is in WRAM.

DumpC: MACRO
    DumpRegisters             \
        0,0,0,\1,0,0,0,0
ENDM


;* LoadC is a one argument shortcut of LoadRegisters which requires a 16 bit
;* address as the argument and loads the C register from the given RAM address.

;* @return A = C

;* Takes 3 bytes and 4 cycles if given address is in HRAM.
;* Takes 4 bytes and 5 cycles if given address is in WRAM.

LoadC: MACRO
    LoadRegisters             \
        0,0,0,\1,0,0,0,0
ENDM


;* DumpD is a one argument shortcut of DumpRegisters which requires a 16 bit
;* address as the argument and dumps the D register to the given RAM address.

;* Destroys A.

;* Takes 3 bytes and 4 cycles if given address is in HRAM.
;* Takes 4 bytes and 5 cycles if given address is in WRAM.

DumpD: MACRO
    DumpRegisters             \
        0,0,0,0,\1,0,0,0
ENDM


;* LoadD is a one argument shortcut of LoadRegisters which requires a 16 bit
;* address as the argument and loads the D register from the given RAM address.

;* @return A = D

;* Takes 3 bytes and 4 cycles if given address is in HRAM.
;* Takes 4 bytes and 5 cycles if given address is in WRAM.

LoadD: MACRO
    LoadRegisters             \
        0,0,0,0,\1,0,0,0
ENDM


;* DumpE is a one argument shortcut of DumpRegisters which requires a 16 bit
;* address as the argument and dumps the E register to the given RAM address.

;* Destroys A.

;* Takes 3 bytes and 4 cycles if given address is in HRAM.
;* Takes 4 bytes and 5 cycles if given address is in WRAM.

DumpE: MACRO
    DumpRegisters             \
        0,0,0,0,0,\1,0,0
ENDM


;* LoadE is a one argument shortcut of LoadRegisters which requires a 16 bit
;* address as the argument and loads the E register from the given RAM address.

;* @return A = E

;* Takes 3 bytes and 4 cycles if given address is in HRAM.
;* Takes 4 bytes and 5 cycles if given address is in WRAM.

LoadE: MACRO
    LoadRegisters             \
        0,0,0,0,0,\1,0,0
ENDM


;* DumpH is a one argument shortcut of DumpRegisters which requires a 16 bit
;* address as the argument and dumps the H register to the given RAM address.

;* Destroys A.

;* Takes 3 bytes and 4 cycles if given address is in HRAM.
;* Takes 4 bytes and 5 cycles if given address is in WRAM.

DumpH: MACRO
    DumpRegisters             \
        0,0,0,0,0,0,\1,0
ENDM


;* LoadH is a one argument shortcut of LoadRegisters which requires a 16 bit
;* address as the argument and loads the H register from the given RAM address.

;* @return A = H

;* Takes 3 bytes and 4 cycles if given address is in HRAM.
;* Takes 4 bytes and 5 cycles if given address is in WRAM.

LoadH: MACRO
    LoadRegisters             \
        0,0,0,0,0,0,\1,0
ENDM


;* DumpL is a one argument shortcut of DumpRegisters which requires a 16 bit
;* address as the argument and dumps the L register to the given RAM address.

;* Destroys A.

;* Takes 3 bytes and 4 cycles if given address is in HRAM.
;* Takes 4 bytes and 5 cycles if given address is in WRAM.

DumpL: MACRO
    DumpRegisters             \
        0,0,0,0,0,0,0,\1
ENDM


;* LoadL is a one argument shortcut of LoadRegisters which requires a 16 bit
;* address as the argument and loads the L register from the given RAM address.

;* @return A = L

;* Takes 3 bytes and 4 cycles if given address is in HRAM.
;* Takes 4 bytes and 5 cycles if given address is in WRAM.

LoadL: MACRO
    LoadRegisters             \
        0,0,0,0,0,0,0,\1
ENDM

;* =============================================================================
;* REMARKS
;* =============================================================================

;* Note that we can only dump and load F register using the stack which is kind
;* of a circumvention. Naturally, this result is very slow compared to dealing
;* with the other registers. If your sandbox does not deliberately require the
;* F register then avoid dumping and loading it.

;* Note that in the LoadRegisters macro we could profit of not using the stack
;* for loading A register because of we are able to jump to F register
;* immediately with "ld hl, sp-2". In the DumpRegisters macro we had to
;* increment HL both was which made them identical in terms of size and CPU
;* cycles. Thus we prefered the A from stack version as it retained A register
;* value if F register was also dumped.


ENDC ;MACRO_REGISTER_INC
