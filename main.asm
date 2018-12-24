;* GB-RNG
;* Copyright (c) 2018 Szieberth Ádám
;* MIT License (see LICENSE file for more info)


;* =============================================================================
;* ABSTRACT
;* =============================================================================

;* The aim of this program is to standardize the random number generators and
;* their testing written for the classic Game Boy hardware. This is the main
;* program which is kept simple as possible yet it provides a standard initial
;* random seed for the RNGs, a visual feedback of the random data. The attached
;* RNG ASM files are only required to name their subroutines as "rand".


;* =============================================================================
;* DEVELOPMENT LOG
;* =============================================================================

;* 1. BACKGROUND AND INITIAL PLANNING
;*    (from 2018-12-19 to 2018-12-24)

;* Recently I finished the prototyping of my first ever game named Octaloop in
;* LÖVE. I planned it for the DMG right from the start: The Game Boy is probably
;* the most successful handheld console ever made with hundreds of millions of
;* units sold, most of them are probably still in working condition boxed and
;* stored somewhere in the home almost forgotten. It is inexpensive. It has
;* AA/AAA batteries which I prefer over the expensive, specific and short
;* lasting Li-ion ones. The GB is robust and durable. In addition I think that
;* most of the greatest games are the simplest ones.

;* After I watched the awesome Ultimate Game Boy Talk video ([ULTIMTALK]) and
;* read some development texts, understand the basics and wrote my Hello World
;* program, the next step would be to have an RNG for my game. After I did some
;* research I realized that from this point on I am on my own. There are no docs
;* nor exmples of GBZ80 RNGs on the internet.

;* I copied the RNG used in 2048-gb ([2048-GB]), made an ASCII characterset
;* tilemap and generated 1kB of random bytes to the VRAM ($9800--$9BFF) to see
;* the data on screen. It was clearly not random. That 2048-gb RNG uses an 8 bit
;* seed which is then XOR'ed with the Divider Register value and the result is
;* returned in A. The RNG routine gets called mainly in the Vblank handler and
;* the randomness of the user input does not contribute to the seed. That RNG
;* might be fine for the 2048 game but I requred a better one.

;* But how would I know that my RNG is good? I read about randomness tests and
;* attempted to do the Diehard tests with the 1kB data I had. Naturally it was
;* under the size limit. I came to the conclusion that I have to transfer at
;* least 30MB data on the link cable to a PC to be able to test my RNG. So I
;* ordered some stuff to be able to do that soon.

;* However, I want to develop my game in the first place so at this point I am
;* fine with a decent RNG which produce a random-like screen and memory dump.
;* And this defined the first milestone (1.0) of GB-RNG and makes the data
;* transfer feature the second milestone (2.0).

;* For 1.0 I want my main file do some initializaton for optional use by the
;* attached RNGs. "Note that GameBoy internal RAM on power up contains random
;* data. All of the GameBoy emulators tend to set all RAM to value $00 on entry.
;* Cart RAM the first time it is accessed on a real GameBoy contains random
;* data. It will only contain known data if the GameBoy code initializes it to
;* some value." [PD] This means that the Game Boy has a nice entropy stored in
;* its RAM after power up.

;* I decided to extract this randomness into a 256 bytes long area on the RAM
;* ($DF00--$DFFF) as part of the initialization process. But as there are many
;* ways to do it, the extractor subroutine used should be also attached from
;* a separate file. It should have its subroutine under the label "extractor"
;* and should extract (BC) bytes into the area starting from (DE). My initial
;* extractor will be doing a simple XOR loop on the target area.

;* The visual representation will be a little modified CP437 print of the RNG
;* generated 360 bytes long data on the 20×18 tiles of the Game Boy display.
;* $00 and $FF will have special tiles which make them differently form the
;* whitespace and the other.

;******************************* CURSOR ****************************************


;*

same PC-BIOS ASCII characterset



 for the Game Boy hardware and after
;* I found it done f
;* The aim of this GBZ80 assembly is to test various random number generators
;* . This main file should be assembled
;* with a random generator for testing.

;* First it copies the initial random bytes from $FF80-$FFFE to $DF80-$DFFE for
;* possible use as entropy source for the RNG. Most code relies on user input
;* times but in my sense that should be only used to make the RNG better and not
;* as the primer entropy. Those kind of RNgs are not useful for someone who
;* wants the game to provide a random map immediately after the start.
;* As part of the initialization, the bytes at $C000-$DF7F, $DFFF and
;* are zeroed.

;* At the current state this demo shows the latin1 characters of the first 20x18
;* bytes of the random data which should sit in $C000-$D000. I plan to make
;* future versions which will send random bytes via the link cable. That would
;* open the possibility to get huge random data provided by a real hardware for
;* diehard testing.

;* This file is a derivative of John Harrison's commented HELLO-WORLD.ASM. I am
;* unsure of who else contibuted to the code but there might be many. Note that
;* I modified that code pretty much.

;* I am new to assembly language and Game Boy programming, so I intend to add
;* a lot of comments, mainly as a note to myself and for other beginners. As I
;* have no knowledge in the topic nor good English knowledge, I will quote most
;* of the time, or summarize the answers of veteran programmers to my questions.
;* I will indicate the sources within square brackets. You find the references
;* in the Appendix. Note that I do not quote the original comments of
;* HELLO-WORLD.ASM though.

;* I hope that soon this code could be a reference and a collection of RNGs for
;* the Game Boy hardware. Let's get started!


;* =============================================================================
;* INCLUDES
;* =============================================================================

;* Most GameBoy assemblers (and most other assembly language assemblers) use a
;* semicolon to indicate that everything following it on a particular line is to
;* be ignored and be treated purely as COMMENTS rather than code.
;*
;* HARDWARE.INC contains the 'Hardware Defines' for our program. This has
;* address location labels for all of the GameBoy Hardware I/O registers. We can
;* 'insert' this file into the present ASM file by using the assembler INCLUDE
;* command:
INCLUDE "HARDWARE.INC"


; ******************************************************************************
; RESTART VECTORS ($0000--$003F)
; ******************************************************************************

;* "The restart vectors are locations that are jumped to if the CPU happens upon
;* a RST $XX opcode. Basically, they are used in routines as a fast CALL, since
;* a normal CALL opcode takes 3 bytes and an RST only takes one byte. Usually,
;* an RST will either have a very short routine, only a few bytes long, since
;* there are only 8 bytes between each RST (eg: $00, $08, $10, $18, etc).
;* Sometimes, if you need a routine a little bit longer than 8 bytes, you can
;* place a JP (jump) at the RST that jumps to your other routine, but TAKE NOTE
;* that jumping is a 3 byte long instruction and defeats the purpose of speed in
;* a RST." [ASMS]

SECTION "Restart Vectors", ROM0[$0000]

;* The SECTION directive RGBASM specific:
;* "Before you can start writing code, you must define a section. This tells the
;* assembler what kind of information follows and, if it is code, where to put
;* it." [RGBDS]
;* For more information please refer to the RGBDS Documentation.

SECTION "RST00", ROM0[$0000]
    ret                         ; 1/16
    ds 7                        ; 7/0   define storage, 7 pad bytes
                                ; b|c   shows the required bytes and CPU cycles

SECTION "RST08", ROM0[$0008]
    ret                         ; 1|16
    ds 7                        ; 7|0

SECTION "RST10", ROM0[$0010]
    ret                         ; 1|16
    ds 7                        ; 7|0

SECTION "RST18", ROM0[$0018]
    ret                         ; 1|16
    ds 7                        ; 7|0

SECTION "RST20", ROM0[$0020]
    ret                         ; 1|16
    ds 7                        ; 7|0

SECTION "RST28", ROM0[$0028]
    ret                         ; 1|16
    ds 7                        ; 7|0

SECTION "RST30", ROM0[$0030]
    ret                         ; 1|16
    ds 7                        ; 7|0

SECTION "RST38", ROM0[$0038]
    ret                         ; 1|16
    ds 7                        ; 7|0


; ******************************************************************************
; INTERRUPT VECTORS ($0040--$00FF)
; ******************************************************************************

;* "The interrupt vectors are locations that the CPU will jump to if certain
;* hardware conditions are met. These hardware conditions are easily
;* enabled/disabled by setting the corresponding bits in the IE (located at
;* $FFFF) register." [ASMS]

;* "When multiple interrupts occur simultaneously, the IE flag of each is set,
;* but only that with the highest priority is started. Those with lower
;* priorities are suspended." [GBPM]

;* "The CPU automatically disables all other interrupts by setting IME=0 when it
;* executes an interrupt. Usually IME remains zero until the interrupt procedure
;* returns (and sets IME=1 by the RETI instruction). However, if you want any
;* other interrupts of lower or higher (or same) priority to be allowed to be
;* executed from inside of the interrupt procedure, then you can place an EI
;* instruction into the interrupt procedure." [PD]

;* "The interrupt processing routine should push the registers during interrupt
;* processing." [GBPM]

;* "Usually, an interrupt vector consists of pushing all 4 registers pairs,
;* followed by a jump to another location." [nitro2k01@gbdev]

SECTION "Interrupt Vectors", ROM0[$0040]

;* "LCD Display Vertical Blanking (Vblank) happens once every frame when the LCD
;* screen has drawn the final scanline. During this short time it’s safe to mess
;* with the video hardware and there won’t be interference." [AD.SKEL]
SECTION	"LCD Display Vertical Blanking", ROM0[$0040]               ; Priority: 1
    reti                        ; 1|16
    ds 7                        ; 7|0

;* "Status Interrupts from LCDC is fired when certain conditions are met in the
;* LCD Status register. Writing to the LCD Status Register, it’s possible to
;* configure which events trigger the Status interrupt. One use is to trigger
;* Horizontal Blank (“h-blank”) interrupts, which occur when there’s a very
;* small window of time between scanlines of the screen, to make a really tiny
;* change to the video memory." [AD.SKEL]
SECTION	"Status Interrupts from LCDC", ROM0[$0048]                 ; Priority: 2
    reti                        ; 1|16
    ds 7                        ; 7|0

;* "Timer Overflow Interrupt is fired when the Game Boy’s 8-bit timer wraps
;* around from 255 to 0. The timer’s update frequency is customizable."
;* [AD.SKEL]
SECTION	"Timer Overflow Interrupt", ROM0[$0050]                    ; Priority: 3
    reti                        ; 1|16
    ds 7                        ; 7|0

;* "Serial Transfer Completion Interrupt is triggered when the a serial link
;* cable transfer has completed sending/receiving a byte of data."  [AD.SKEL]
SECTION	"Serial Transfer Completion Interrupt", ROM0[$0058]        ; Priority: 4
    reti                        ; 1|16
    ds 7                        ; 7|0

;* End of Input Signal for ports P10-P13 (Joypad) Interrupt is triggered when
;* any of the six buttons is pressed on the joypad. "The primary purpose of this
;* interrupt is to break the Game Boy from its low-power standby state, and
;* isn’t terribly useful for much else." [AD.SKEL]
SECTION	"End of Input Signal for ports P10-P13 Interrupt", ROM0[$0060]
                                                                   ; Priority: 5
    reti                        ; 1|16
    ds 7                        ; 7|0

;* "After the interrupt vectors there is a small area with some free space which
;* you can use for whatever you want." [AD.SKEL]
;*
;* This space can be used for anything you want including an uncommon longer
;* joypad interrupt handler or anything else. You might also enlarge it with the
;* previous seven pad area if you are fine with the plain reti joypad handler.
;* Moreover, the whole $0000--$00FF area can be used freely if your program
;* does not use restarts and disables interrupts. You can also do relative jumps
;* over required restart and/or interrupt handlers if necessary.
SECTION	"Free Space from $068", ROM0[$0068]
    ds 152                      ; 152|0 $0068--$00FF


; ******************************************************************************
; ROM REGISTRATION DATA ($0100--$014F)
; ******************************************************************************

;* The first part of the ROM is the 80 bytes long registration data with
;* information regarding the game title and Game Boy software specifications.
;* This 80 bytes are located at $0100--$014F on the ROM. (The $ before a number
;* indicates that the number is a hex value.)
SECTION	"ROM Registration Data", ROM0[$0100]  ; ends with $014F

;* 1. Code execution starting point and start address: The program starts after
;*    Initial Program Load (IPL) is run on the CPU. The standard first two
;*    commands are usually always a NOP (NO Operation) and then a JP (Jump)
;*    command. This JP command should 'jump' to the start of user code. It jumps
;*    over the remaining ROM Registration Data, usually to $0150. The jump
;*    command length is three byte with bytes 2 and 3 containing the address,
;*    which is in this case the start address of the program. As the Game Boy
;*    CPU is little-endian so the low byte of the starting address is stored
;*    first, then the high byte.
    nop                         ; 1|4   $00
    jp begin                    ; 3|16  $C3 $Lo $Hi (begin is a label which will
                                ;       get replaced with its start address by
                                ;       the assembler)

;* 2. Nintendo logo:  "For a piece of software to run on a Game Boy, it must
;*    contain a copy of Nintendo's logo identical to the one in the console's
;*    internal ROM, and that logo will be displayed at startup; this was
;*    presumably done for similar reasons as Sega's TMSS, in that it forces any
;*    unlicensed producer of cartridges (whether a pirate or an otherwise
;*    legitimate unlicensed developer) to include the Nintendo logo in their
;*    games, theoretically committing trademark infringement in the process. ...
;*    But, fortunately for unlicensed developers, Sega's TMSS didn't hold up in
;*    court (in the US anyway), which I'd guess rendered Nintendo's Game Boy
;*    efforts largely pointless too." [GGLOGO]
;*
;*    The included HARDWARE.INC have a "NINTENDO_LOGO" macro to make this easy.
;*    Note that RGBFIX (-v) als set this data properly.
SECTION	"Nintendo Logo", ROM0[$0104]
    NINTENDO_LOGO               ; 30|0

;* 3. Game title: "The game title is an ASCII code up to 11 characters. Use code
;*    $20 for a space and code $00 for all unused areas in the game title."
;*    [GBPM] The following (uppercase) characters are allowed
;*    (between >> and <<):
;*      >> !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_<<
;*    Note that forthslashes (\) might be treated as ¥ signs.
SECTION	"Game Title", ROM0[$0134]
    DB "RNG DEMO"               ; 8|0
       ;0123456789A
    DS 3                        ; 3|0   unused area, three bytes of zeroes ($00)

;* 4. Game code: Ideally the Game Code is assigned by Nintendo. Its allowed
;*    ASCII characters are the same as for the game title. Best practice for
;*    unlicensed products is to fill this area with spaces.
SECTION	"Game Code", ROM0[$013F]
    DB "    "                   ; 4|0
       ;0123

;* 5. CGB Support Code: "This value distinguishes between games that are CGB
;*    (Game Boy Color) compatible, and those that are not. Valid values:
;*    • $00: CGB Incompatible. A program which does not use CGB functions, but
;*           operates with both CGB and DMG (Monochrome).
;*    • $80: CGB Compatible. A program which uses CGB functions, and operates
;*           with both CGB and DMG.
;*    • $C0: CGB Exclusive. A program which uses CGB functions, but will only
;*           operate on a Game Boy Color unit (not on DMG/MGB). If a user
;*           attempts to play this software on Game Boy, a screen must be
;*           displayed telling the user that the game must be played on Game Boy
;*           Color." [GBPM]
SECTION	"CGB Support Code", ROM0[$0143]
    DB $00                      ; 1|0

;* 6. Maker Code: 2-digit uppercase ASCII code assigned by Nintendo. Best
;*    practice for unlicensed products is to fill this area with spaces.
SECTION	"Maker Code", ROM0[$0144]
    DB "  "                     ; 2|0
       ;01

;* 7. SGB Support Code: Specifies whether the game supports SGB functions. Valid
;*    values:
;*    • $00: No Super Game Boy Functions
;*    • $03: Uses Super Game Boy Functions
;*    In order to use Super Game Boy Functions, the Legacy Maker Code must be
;*    $33. [GBPM]
SECTION	"SGB Support Code", ROM0[$0146]
    DB	$00	                    ; 1|0

;* 8. Software Type (Cartridge Type): "Specifies which Memory Bank Controller
;*    (if any) is used in the cartridge, and if further external hardware exists
;*    in the cartridge." [PD] The valid values are listed in the first block of
;*    the "Cart related" section of HARDWARE.INC.
SECTION	"Software Type", ROM0[$0147]
    DB CART_ROM                 ; 1|0

;* 9. ROM Size: "Specifies the ROM Size of the cartridge." [PD] The valid values
;*    are listed in the second block of the "Cart related" section of
;*    HARDWARE.INC.
SECTION	"ROM Size", ROM0[$0148]
    DB	CART_ROM_256K           ; 1|0

;* 10. External RAM Size: "Specifies the size of the external RAM in the
;*     cartridge (if any)." [PD] The valid values are listed in the third block
;*     of the "Cart related" section of HARDWARE.INC.
SECTION	"External RAM Size", ROM0[$0149]
    DB	CART_RAM_NONE           ; 1|0

;* 11. Destination Code: "Specifies if this version of the game is supposed to
;*     be sold in Japan, or anywhere else. Only two values are defined:" [PD]
;*     • $00: Japan
;*     • $01: All Others
SECTION	"Destination Code", ROM0[$014A]
    DB	$01                     ; 1|0

;* 12. Legacy Maker Code: "Specifies the games company/publisher code in range
;*     $00--$FF." [PD] A value of $33 signalizes that the new Maker Code in
;*     header bytes $0144--$0145 is used instead. Note that Super GameBoy
;*     functions will not work if this value is not $33. [PD]
SECTION	"Legacy Maker Code", ROM0[$014B]
    DB	$33                     ; 1|0

;* 13. Mask ROM Version N0.: "The mask ROM version number starts from $00 and
;*     increases by 1 for each revised version sent after starting production."
;*     [GBPM] Therefore this value is usually $00.
SECTION	"Mask ROM Version N0.", ROM0[$014C]
    DB	$00                     ; 1|0

;* 14. Complement Check (Header Checksum): "After all the registration data has
;*     been entered ($0134--$014C), add $19 to the sum of the data stored at
;*     addresses $0134 through $014C and store the complement value of the
;*     resulting sum.
;*             ($0134) + ($0135) + ... + ($014C) + $19 + ($014D) = $00
;*     " [GBPM]
;*     We usually use RGBFIX (-v) to set this value for us, thus, set it to $00.
SECTION	"Complement Check", ROM0[$014D]
    DB	$00                     ; 1|0

;* 15. Check Sum Hi and Lo (Global Checksum): "Contains a 16 bit checksum (upper
;*     byte first) across the whole cartridge ROM. Produced by adding all bytes
;*     of the cartridge (except for the two checksum bytes). The Gameboy doesn't
;*     verify this checksum." [PD]
;*     We usually use RGBFIX (-v) to set this value for us, thus, set it to $00.
SECTION	"Check Sum Hi and Lo", ROM0[$014E]
    DW	$0000                   ; 2|0






























H_RNG1 EQU $FF80

; Macro that pauses until VRAM available.

lcd_WaitVRAM: MACRO
        ld      a,[rSTAT]       ; <---+
        and     STATF_BUSY      ;     |
        jr      nz,@-4          ; ----+
        ENDM



;  Next we need to include the standard GameBoy ROM header
; information that goes at location $0100 in the ROM. (The
; $ before a number indicates that the number is a hex value.)
;
;  ROM location $0100 is also the code execution starting point
; for user written programs. The standard first two commands
; are usually always a NOP (NO Operation) and then a JP (Jump)
; command. This JP command should 'jump' to the start of user
; code. It jumps over the ROM header information as well that
; is located at $104.
;
;  First, we indicate that the following code & data should
; start at address $100 by using the following SECTION assembler
; command:

SECTION	"start",ROM0[$100]
    nop
    jp	begin

;  To include the standard ROM header information we
; can just use the macro ROM_HEADER. We defined this macro
; earlier when we INCLUDEd "gbhw.inc".
;
;  The ROM_NOMBC just suggests to the complier that we are
; not using a Memory Bank Controller because we don't need one
; since our ROM won't be larger than 32K bytes.
;
;  Next we indicate the cart ROM size and then the cart RAM size.
; We don't need any cart RAM for this program so we set this to 0K.

; ****************************************************************************************
; ROM HEADER and ASCII character set
; ****************************************************************************************

    ; $0104-$0133 Nintendo logo (hardware.inc MACRO)
    NINTENDO_LOGO

    ; $0134-$013E title
    DB	"RNG DEMO   "
        ;0123456789A

    ; $013F-$0142 manufacturer code (leave blank)
    DB	"    "
        ;0123

    ; $0143 CGB flag
    DB	$00	; $00 - DMG
            ; $80 - DMG/GBC
            ; $C0 - GBC Only

    ; $0144-$0145 new licensee code
    DB	$00,$00

    ; $0146 SGB flag
    DB	$00	; $00 = No SGB functions
             ; $03 = Supports SGB functions

    ; $0147 cartridge type
    DB	CART_ROM

    ; $0148 ROM size
    DB	CART_ROM_256K

    ; $0149 RAM size
    DB	CART_RAM_NONE

    ; $014A destination code
    DB	$01	; $01 - All others
            ; $00 - Japan

    ; $014B old licensee code (must be $33)
    DB	$33

    ; $014C mask ROM version (handled by RGBFIX)
    DB	$00

    ; $014D complement check (handled by RGBFIX)
    DB	$00

    ; $014E-$014F cartridge checksum (handled by RGBFIX)
    DW	$00

;  The NOP and then JP located at $100 in ROM are executed
; which causes the the following code to be executed next.

; ****************************************************************************************
; Main code Initialization:
; set the stack pointer, enable interrupts, set the palette, set the screen relative to the window
; copy the ASCII character table, clear the screen
; ****************************************************************************************
SECTION	"main",ROM0[$150]
begin:
; First, it's a good idea to Disable Interrupts
; using the following command. We won't be using
; interrupts in this example so we can leave them off.

    di

;  Next, we should initialize our stack pointer. The
; stack pointer holds return addresses (among other things)
; when we use the CALL command so the stack is important to us.
;
;  The CALL command is similar to executing
; a procedure in the C & PASCAL languages.
;
; We shall set the stack to the top of high ram + 1.
;
    ld	sp, $ffff		; set the stack pointer to highest mem location we can use + 1

;  Here we are going to setup the background tile
; palette so that the tiles appear in the proper
; shades of grey.
;
;  To do this, we need to write the value %11100100 to the
; memory location $ff47. In the 'gbhw.inc' file we
; INCLUDEd there is a definition that rBGP=$ff47 so
; we can use the rGBP label to do this
;
;  The first instruction loads the value %11100100 into the
; 8-bit register A and the second instruction writes
; the value of register A to memory location $ff47.

init:
    ld	a, %11100100 	; Window palette colors, from darkest to lightest
    ld	[rBGP], a		; CLEAR THE SCREEN

    ld hl, $C000
.loop
    ld a, 0
    ld [hli], a
    ld a, h
    cp $e0
    jr nz, .loop

    ld hl, $ff81
    jr .loop2
.loop2rng
    ld a, [H_RNG1]
    ld b, a
    ld a, [hl]
    xor b
    ld [H_RNG1], a
.loop2
    ld a, 0
    ld [hli], a
    ld a, l
    cp $FE
    jr nz, .loop2rng

    ld a, 0
    ld [H_RNG1], a ; test




;  Here we are setting the X/Y scroll registers
; for the tile background to 0 so that we can see
; the upper left corner of the tile background.
;
;  Think of the tile background RAM (which we usually call
; the tile map RAM) as a large canvas. We draw on this
; 'canvas' using 'paints' which consist of tiles and
; sprites (we will cover sprites in another example.)
;
;  We set the scroll registers to 0 so that we can
; view the upper left corner of the 'canvas'.

    ld	a,0			; SET SCREEN TO TO UPPER RIGHT HAND CORNER
    ld	[rSCX], a
    ld	[rSCY], a

;  Next we shall turn the Liquid Crystal Display (LCD)
; off so that we can copy data to video RAM. We can
; copy data to video RAM while the LCD is on but it
; is a little more difficult to do and takes a little
; bit longer. Video RAM is not always available for
; reading or writing when the LCD is on so it is
; easier to write to video RAM with the screen off.
;
;  To turn off the LCD we do a CALL to the StopLCD
; subroutine at the bottom of this file. The reason
; we use a subroutine is because it takes more than
; just writing to a memory location to turn the
; LCD display off. The LCD display should be in
; Vertical Blank (or VBlank) before we turn the display
; off. Weird effects can occur if you don't wait until
; VBlank to do this and code written for the Super
; GameBoy won't work sometimes you try to turn off
; the LCD outside of VBlank.

    call	StopLCD		; YOU CAN NOT LOAD $8000 WITH LCD ON

;  In order to display any text on our 'canvas'
; we must have tiles which resemble letters that
; we can use for 'painting'. In order to setup
; tile memory we will need to copy our font data
; to tile memory using the routine 'mem_CopyMono'
; found in the 'memory.asm' library we INCLUDEd
; earlier.
;
;  For the purposes of the 'mem_CopyMono' routine,
; the 16-bit HL register is used as a source memory
; location, DE is used as a destination memory location,
; and BC is used as a data length indicator.

;    ld hl, _VRAM                ; First tile will be empty so I will start with
;    ld bc, 16                   ; an empty screen.
;    xor a                       ; a = $00
;    call mem_Set
;    ld a, h                     ; copy hl to de
;    ld d, a
;    ld a, l
;    ld e, a
    ld de, _VRAM
    ld	hl, TileData	; $8000
;    ld	bc, 16*16 		; 16 characters, each with 16 bytes of display data
    ld	bc, 256*16 		; 16+1 characters, each with 16 bytes of display data
    call	mem_Copy	; load tile data

; We turn the LCD on. Parameters are explained in the I/O registers section of The GameBoy reference under I/O register LCDC
    ld	a, LCDCF_ON|LCDCF_BG8000|LCDCF_BG9800|LCDCF_BGON|LCDCF_OBJ16|LCDCF_OBJOFF
    ld	[rLCDC], a

; Next, we clear our 'canvas' to all white by
; 'setting' the canvas to ascii character $20
; which is a white space.

;   ld	a, 32		; ASCII FOR BLANK SPACE
;   ld	hl, _SCRN0
;   ld	bc, SCRN_VX_B * SCRN_VY_B
;   call	mem_SetVRAM


; ****************************************************************************************
; Main code:
; Print a character string in the middle of the screen
; ****************************************************************************************
; Now we need to paint the message
; " Hello World !" onto our 'canvas'. We do this with
; one final memory copy routine call.

vramdraw:
    ld	bc, $0400 + 1; 16x16 tiles
    ld	de, _SCRN0
	jr	.skip
.vramloop
    di
    push bc
    call GetRNG
    lcd_WaitVRAM
    ld a, [H_RNG1]
;    ld b, a
;    and %00001111
;    inc a
;    ld	[de],a
;    inc de
;    ld a, b
;    swap a
;    and %00001111
;    inc a
    ld	[de],a
    pop bc
;    dec bc
    ei
	inc	de
.skip
    dec	bc
    ld a, c
    and a                       ; compare to 0
	jr	nz,.vramloop
	ld a, b
    and a
	jr	nz,.vramloop

; ****************************************************************************************
; Prologue
; Wait patiently 'til somebody kills you
; ****************************************************************************************
; Since we have accomplished our goal, we now have nothing
; else to do. As a result, we just Jump to a label that
; causes an infinite loop condition to occur.
wait:
    halt
    nop
    jr	wait

; ****************************************************************************************
; hard-coded data
; ****************************************************************************************
Title:
    DB	"Hello World !"
TitleEnd:
    nop
; ****************************************************************************************
; StopLCD:
; turn off LCD if it is on
; and wait until the LCD is off
; ****************************************************************************************
StopLCD:
        ld      a,[rLCDC]
        rlca                    ; Put the high bit of LCDC into the Carry flag
        ret     nc              ; Screen is off already. Exit.

; Loop until we are in VBlank

.wait:
        ld      a,[rLY]
        cp      145             ; Is display on scan line 145 yet?
        jr      nz,.wait        ; no, keep waiting

; Turn off the LCD

        ld      a,[rLCDC]
        res     7,a             ; Reset bit 7 of LCDC
        ld      [rLCDC],a

        ret

;  Next, let's actually include font tile data into the ROM
; that we are building. We do this by invoking the chr_IBMPC1
; macro that was defined earlier when we INCLUDEd "ibmpc1.inc".
;
;  The 1 & 8 parameters define that we want to include the
; whole IBM-PC font set and not just parts of it.
;
;  Right before invoking this macro we define the label
; TileData. Whenever a label is defined with a colon
; it is given the value of the current ROM location.
;  As a result, TileData now has a memory location value that
; is the same as the first byte of the font data that we are
; including. We shall use the label TileData as a "handle" or
; "reference" for locating our font data.

TileData:
    ;INCBIN "font.bin"
    INCBIN "ibmpc1.bin"

        PUSHS           ; Push the current section onto assember stack.

;***************************************************************************
;*
;* mem_Set - "Set" a memory region
;*
;* input:
;*    a - value
;*   hl - pMem
;*   bc - bytecount
;*
;***************************************************************************
mem_Set::
	inc	b
	inc	c
	jr	.skip
.loop	ld	[hl+],a
.skip	dec	c
	jr	nz,.loop
	dec	b
	jr	nz,.loop
	ret

;***************************************************************************
;*
;* mem_Copy - "Copy" a memory region
;*
;* input:
;*   hl - pSource
;*   de - pDest
;*   bc - bytecount
;*
;***************************************************************************
mem_Copy::
	inc	b
	inc	c
	jr	.skip
.loop	ld	a,[hl+]
	ld	[de],a
	inc	de
.skip	dec	c
	jr	nz,.loop
	dec	b
	jr	nz,.loop
	ret

;***************************************************************************
;*
;* mem_SetVRAM - "Set" a memory region in VRAM
;*
;* input:
;*    a - value
;*   hl - pMem
;*   bc - bytecount
;*
;***************************************************************************
mem_SetVRAM::
	inc	b
	inc	c
	jr	.skip
.loop   push    af
        di
        lcd_WaitVRAM
        pop     af
        ld      [hl+],a
        ei
.skip	dec	c
	jr	nz,.loop
	dec	b
	jr	nz,.loop
	ret

;***************************************************************************
;*
;* mem_CopyVRAM - "Copy" a memory region to or from VRAM
;*
;* input:
;*   hl - pSource
;*   de - pDest
;*   bc - bytecount
;*
;***************************************************************************
mem_CopyVRAM::
	inc	b
	inc	c
	jr	.skip
.loop   di
        lcd_WaitVRAM
        ld      a,[hl+]
	ld	[de],a
        ei
	inc	de
.skip	dec	c
	jr	nz,.loop
	dec	b
	jr	nz,.loop
	ret

        POPS           ; Pop the current section off of assember stack.

GetRNG:
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

;* [AD.SKEL]    Assembly Digest: Tutorial: Making an Empty Game Boy ROM
;*              http://assemblydigest.tumblr.com/post/77198211186

;* [ASMS]       Randy Mongenel (Duo): ASMSchool
;*              http://gameboy.mongenel.com/asmschool.html

;* [AWESOME]    Awesome Game Boy Development
;*              https://github.com/gbdev/awesome-gbdev

;* [GBCPUMan]   DP: Game Boy™ CPU Manual v1.01
;*              http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf

;* [GBPM]       Game Boy Programming Manual
;*              https://archive.org/download/GameBoyProgManVer1.1/GameBoyProgManVer1.1.pdf

;* [GGLOGO]     Taizou: Go Go Logo
;*              http://fuji.drillspirits.net/?post=87

;* [PD]         Martin Korth et al.: Pan Docs
;*              http://gbdev.gg8.se/wiki/articles/Pan_Docs

;* [RGBDS]      RGBDS Documentation
;*              https://rednex.github.io/rgbds/

;* [ULTIMTALK]  Michael Steil: The Ultimate Game Boy Talk (VIDEO)
;*              https://media.ccc.de/v/33c3-8029-the_ultimate_game_boy_talk

;* [WTI.RAND]   WiniTI: Z80 Routines:Math:Random
;*              http://wikiti.brandonw.net/index.php?title=Z80_Routines:Math:Random

;* [XY@gbdev]   gbdev post by XY
;*              https://discord.gg/gpBxq85

;* [Z80BITS]    Z80 Bits: Random Number Generators
;*              http://www.msxdev.com/sources/external/z80bits.html#3