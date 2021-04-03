; the loader is a small 65816 program suitable for the cpld boot rom (est 64 bytes..)
; it will enable native mode, load the first item from spi flash at sel2 and jump to the start address
.setcpu         "65816"
.smart          on
.autoimport     on
.case           on
.debuginfo      off

.define SPI     $FF80
.define SPIDATA SPI
.define SPISTAT SPI+1
.define SPI65CTRL SPISTAT 
.define SPIDIV  SPI+2
.define SPISEL  SPI+3
.define SLAVE_SEL $04
.define SPI65_STAT_BSY $20
.define SPI65_STAT_DONE $80
.define CMD_READ $03

.segment        "RODATA"


_reset:
        ; slave sel2
        lda #SLAVE_SEL
        sta SPISEL
; send cmd read
        ldx #3;
l1:     lda _read,x
        jsr SENDBYTE
        dex
        bpl l1
;load first 5 bytes (size+start address)
        ldx #4
l2:     jsr READBYTE
        sta $7E,x
        dex 
        bpl l2

;  ZP address after loading 5 bytes
;    $7E $7F size of block
;    $80 $81 start address of block
;
; not needed, y is zero after reset
;        ldy #$0

        ; to native mode
        clc    ;clear carry to zero.
        xce    ;exchange (swap) carry with the emulation bit.
        sep #$20 ; akku 8 bit

        ldx $7E 
_copy:
        jsr READBYTE
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

READBYTE:
	lda #SPI65_STAT_BSY
l3:	bit SPI65CTRL
	bne l3				; wait until not busy
	lda #SPI65_STAT_DONE
	sta SPIDATA	; trigger transfer
l4:	bit SPI65CTRL
	bpl l4				; wait till transfer done
        tya
	lda SPIDATA
	rts
	

SENDBYTE:
	tay
	lda #SPI65_STAT_BSY
l5:	bit SPI65CTRL
	bne l5				; wait until not busy
	sty SPIDATA	; trigger transfer
	rts
	


_read:
.byte $0, $0, $0, $3

_irq:
_break:
        rti

.segment        "VECTORS"
        .addr  _break
        .addr  _reset        
        .addr  _irq
