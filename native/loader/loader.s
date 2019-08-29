; the loader is a small program which will be present in the cpld
; it will enable native mode, load the first blocks from spi flash at sel0 and jump there to start
.setcpu         "65816"
.smart          on
.autoimport     on
.case           on
.debuginfo      off

.define SPI     $DE00
.define SPIDATA SPI
.define SPISTAT SPI+1
.define SPIDIV  SPI+2
.define SPISEL  SPI+3
.define SLAVE_SEL $04

.define CMD_READ $03

.segment        "RODATA"


_reset:
        ; to native mode
        clc    ;clear carry to zero.
        xce    ;exchange (swap) carry with the emulation bit.
        rep #$10
        sep #$20
        jsr _sel
        lda #CMD_READ
        sta SPIDATA
        lda #$00
        sta SPIDATA
        lda #$00
        sta SPIDATA
        lda #$00
        sta SPIDATA ; start reading at addr 0x00 (byte )

        lda SPIDATA ; load target msb
        tax         ; target msb is now in x
        lda SPIDATA
        sta $81
        lda SPIDATA
        sta $80

        lda SPIDATA ; load size of block
        sta $83
        lda SPIDATA ; load size of block
        sta $82
        ldy #$0
        ldx $82 
_copy:
        lda SPIDATA
        sta ($80),y
        iny
        dex
        bne _copy
        jsr _unsel
        ; back to emulation mode
        sec    ;set carry to one.
        xce    ;exchange (swap) carry with the emulation bit.
        jmp ($80)
_sel:
        lda #SLAVE_SEL
        sta SPISEL
        rts
_unsel:
        lda #$0
        sta SPISEL
        rts

_irq:
_break:
        rti

.segment        "VECTORS"
        .addr  _break
        .addr  _reset        
        .addr  _irq
