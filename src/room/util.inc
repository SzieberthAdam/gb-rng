SECTION	"Display Utilities", ROM0


; Converts the value to a tile with the appropriate hexadecimal number.
; @param a: The number whose tile we want. Must be within 0-15 or expect weird
;     results.
; @return a: the tile number
hex_tile_of_a:
    cp a, 10                    ; 2|2
    jr nc, .inrangeAF           ; 2|3/2
.inrange09
    add a, $30                  ; 2|2
    ret                         ; 1|4
.inrangeAF
    add a, $37                  ; 2|2
    ret                         ; 1|4

                                ; 10|10/11 (11 cycles if value is in A-F)
