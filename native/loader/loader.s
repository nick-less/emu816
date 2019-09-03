; the loader is a small program which will be present in the cpld
; it will enable native mode, load the first blocks from spi flash at sel0 and jump there to start
.setcpu         "65816"
.smart          on
.autoimport     on
.case           on
.debuginfo      off

.define SPI     $FF80
.define SPIDATA SPI
.define SPISTAT SPI+1
.define SPIDIV  SPI+2
.define SPISEL  SPI+3
.define SLAVE_SEL $04

.define CMD_READ $03

.segment        "RODATA"


_reset:
        ; slave sel0
        lda #SLAVE_SEL
        sta SPISEL
; send cmd read
        ldx #3;
l1:     lda _read,x
        sta SPIDATA
        dex
        bpl l1
;load first 5 bytes (size+start address)
        ldx #4
l2:     lda SPIDATA
        sta $7E,x
        dex 
        bpl l2

; not need, y is zero after reset
;        ldy #$0

        ; to native mode
        clc    ;clear carry to zero.
        xce    ;exchange (swap) carry with the emulation bit.
        sep #$20 ; akku 8 bit

        ldx $7E 
_copy:
        lda SPIDATA
        sta ($80),y
        iny
        dex
        bne _copy
        ; slave unsel
        lda #$0
        sta SPISEL
        ; back to emulation mode
        sec    ;set carry to one.
        xce    ;exchange (swap) carry with the emulation bit.
        jmp ($80)

_read:
.byte $00, $00, $00, $03

_irq:
_break:
        rti

.segment        "VECTORS"
        .addr  _break
        .addr  _reset        
        .addr  _irq
