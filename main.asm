;--------------------------------------------------------
; File Created by SDCC : free open source ISO C Compiler
; Version 4.5.10 #15720 (MINGW32)
;--------------------------------------------------------
	.module main
	
	.optsdcc -mz180 sdcccall(1)
	.hd64
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl _main
	.globl _lcd_put_hex2
	.globl _lcd_put_hex1
	.globl _lcd_print
	.globl _lcd_printChar
	.globl _lcd_setCursor
	.globl _lcd_clear
	.globl _lcd_init
	.globl _delay_ms
	.globl _delay_us
	.globl _asci1_init
	.globl _asci1_getc
	.globl _asci1_put_hex2
	.globl _asci1_put_hex1
	.globl _asci1_puts
	.globl _asci1_putc
	.globl _asci0_init
	.globl _asci0_getc
	.globl _asci0_put_hex2
	.globl _asci0_put_hex1
	.globl _asci0_puts
	.globl _asci0_putc
;--------------------------------------------------------
; special function registers
;--------------------------------------------------------
_CNTLA0	=	0x0000
_CNTLA1	=	0x0001
_CNTLB0	=	0x0002
_CNTLB1	=	0x0003
_STAT0	=	0x0004
_STAT1	=	0x0005
_TDR0	=	0x0006
_TDR1	=	0x0007
_RDR0	=	0x0008
_RDR1	=	0x0009
_CNTR	=	0x000a
_TRDR	=	0x000b
_TMDR0L	=	0x000c
_TMDR0H	=	0x000d
_RLDR0L	=	0x000e
_RLDR0H	=	0x000f
_TCR	=	0x0010
_TMDR1L	=	0x0014
_TMDR1H	=	0x0015
_RLDR1L	=	0x0016
_RLDR1H	=	0x0017
_FRC	=	0x0018
_SAR0L	=	0x0020
_SAR0H	=	0x0021
_SAR0B	=	0x0022
_DAR0L	=	0x0023
_DAR0H	=	0x0024
_DAR0B	=	0x0025
_BCR0L	=	0x0026
_BCR0H	=	0x0027
_MAR1L	=	0x0028
_MAR1H	=	0x0029
_MAR1B	=	0x002a
_IAR1L	=	0x002b
_IAR1H	=	0x002c
_BCR1L	=	0x002e
_BCR1H	=	0x002f
_DSTAT	=	0x0030
_DMODE	=	0x0031
_DCNTL	=	0x0032
_IL	=	0x0033
_ITC	=	0x0034
_RCR	=	0x0036
_CBR	=	0x0038
_BBR	=	0x0039
_CBAR	=	0x003a
_OMCR	=	0x003e
_ICR	=	0x003f
_LCD_CMD	=	0x0040
_LCD_DATA	=	0x0041
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _DATA
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _INITIALIZED
;--------------------------------------------------------
; absolute external ram data
;--------------------------------------------------------
	.area _DABS (ABS)
;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
	.area _HOME
	.area _GSINIT
	.area _GSFINAL
	.area _GSINIT
;--------------------------------------------------------
; Home
;--------------------------------------------------------
	.area _HOME
	.area _HOME
;--------------------------------------------------------
; code
;--------------------------------------------------------
	.area _CODE
;c:\program files (x86)\sdcc\include\z180\z180.h:40: static void _ENABLE_Z180_ASSEMBLER_(void) __naked { __asm .hd64 __endasm; }
;	---------------------------------
; Function _ENABLE_Z180_ASSEMBLER_
; ---------------------------------
__ENABLE_Z180_ASSEMBLER_:
	.hd64	
;main.c:13: void asci0_putc(char c)
;	---------------------------------
; Function asci0_putc
; ---------------------------------
_asci0_putc::
	ld	c, a
;main.c:15: while ((STAT0 & STAT0_TDRE) == 0) ; // wait for TX buffer empty
00101$:
	in0	a, (_STAT0)
	bit	1, a
	jr	Z, 00101$
;main.c:16: TDR0 = c;
	ld	a, c
	out0	(_TDR0), a
;main.c:17: }
	ret
;main.c:19: void asci0_puts(const char* s)
;	---------------------------------
; Function asci0_puts
; ---------------------------------
_asci0_puts::
;main.c:21: while(*s)
00101$:
	ld	a, (hl)
	or	a, a
	ret	Z
;main.c:23: asci0_putc(*s++);
	inc	hl
	push	hl
	call	_asci0_putc
	pop	hl
;main.c:25: }
	jr	00101$
;main.c:27: void asci0_put_hex1(const unsigned char n)
;	---------------------------------
; Function asci0_put_hex1
; ---------------------------------
_asci0_put_hex1::
;main.c:31: asci0_putc(n + '0');
	ld	c, a
;main.c:29: if (n < 10)
	sub	a, #0x0a
	jr	NC, 00102$
;main.c:31: asci0_putc(n + '0');
	ld	a, c
	add	a, #0x30
	jp	_asci0_putc
00102$:
;main.c:35: asci0_putc(n + 'A' - 10);
	ld	a, c
	add	a, #0x37
;main.c:37: }
	jp	_asci0_putc
;main.c:39: void asci0_put_hex2(const unsigned char h)
;	---------------------------------
; Function asci0_put_hex2
; ---------------------------------
_asci0_put_hex2::
	ld	c, a
;main.c:41: asci0_put_hex1(h >> 4);
	ld	e, c
	ld	d, #16
	mlt	de
	push	bc
	ld	a, d
	call	_asci0_put_hex1
	pop	bc
;main.c:42: asci0_put_hex1(h & 0xf);
	ld	a, c
	and	a, #0x0f
;main.c:43: }
	jp	_asci0_put_hex1
;main.c:45: char asci0_getc(void)
;	---------------------------------
; Function asci0_getc
; ---------------------------------
_asci0_getc::
;main.c:47: while ((STAT0 & STAT0_RDRF) == 0) ; // wait for RX data ready
00101$:
	in0	a, (_STAT0)
	rlca
	jr	NC, 00101$
;main.c:48: return RDR0;
	in0	a, (_RDR0)
;main.c:49: }
	ret
;main.c:51: void asci0_init(void)
;	---------------------------------
; Function asci0_init
; ---------------------------------
_asci0_init::
;main.c:57: CNTLA0 = CNTLA0_RE|CNTLA0_RTS0|CNTLA0_TE|CNTLA0_MOD2;
	ld	a, #0x74
	out0	(_CNTLA0), a
;main.c:63: CNTLB0 = CNTLB0_SS0;
	ld	a, #0x01
	out0	(_CNTLB0), a
;main.c:66: (void)STAT0;
	in0	a, (_STAT0)
;main.c:67: }
	ret
;main.c:72: void asci1_putc(char c)
;	---------------------------------
; Function asci1_putc
; ---------------------------------
_asci1_putc::
	ld	c, a
;main.c:74: while ((STAT1 & STAT1_TDRE) == 0) ; // wait for TX buffer empty
00101$:
	in0	a, (_STAT1)
	bit	1, a
	jr	Z, 00101$
;main.c:75: TDR1 = c;
	ld	a, c
	out0	(_TDR1), a
;main.c:76: }
	ret
;main.c:78: void asci1_puts(const char* s)
;	---------------------------------
; Function asci1_puts
; ---------------------------------
_asci1_puts::
;main.c:80: while(*s)
00101$:
	ld	a, (hl)
	or	a, a
	ret	Z
;main.c:82: asci1_putc(*s++);
	inc	hl
	push	hl
	call	_asci1_putc
	pop	hl
;main.c:84: }
	jr	00101$
;main.c:86: void asci1_put_hex1(const unsigned char n)
;	---------------------------------
; Function asci1_put_hex1
; ---------------------------------
_asci1_put_hex1::
;main.c:90: asci1_putc(n + '0');
	ld	c, a
;main.c:88: if (n < 10)
	sub	a, #0x0a
	jr	NC, 00102$
;main.c:90: asci1_putc(n + '0');
	ld	a, c
	add	a, #0x30
	jp	_asci1_putc
00102$:
;main.c:94: asci1_putc(n + 'A' - 10);
	ld	a, c
	add	a, #0x37
;main.c:96: }
	jp	_asci1_putc
;main.c:98: void asci1_put_hex2(const unsigned char h)
;	---------------------------------
; Function asci1_put_hex2
; ---------------------------------
_asci1_put_hex2::
	ld	c, a
;main.c:100: asci1_put_hex1(h >> 4);
	ld	e, c
	ld	d, #16
	mlt	de
	push	bc
	ld	a, d
	call	_asci1_put_hex1
	pop	bc
;main.c:101: asci1_put_hex1(h & 0xf);
	ld	a, c
	and	a, #0x0f
;main.c:102: }
	jp	_asci1_put_hex1
;main.c:104: char asci1_getc(void)
;	---------------------------------
; Function asci1_getc
; ---------------------------------
_asci1_getc::
;main.c:106: while ((STAT1 & STAT1_RDRF) == 0) ; // wait for RX data ready
00101$:
	in0	a, (_STAT1)
	rlca
	jr	NC, 00101$
;main.c:107: return RDR1;
	in0	a, (_RDR1)
;main.c:108: }
	ret
;main.c:110: void asci1_init(void)
;	---------------------------------
; Function asci1_init
; ---------------------------------
_asci1_init::
;main.c:116: CNTLA1 = CNTLA1_RE|CNTLA1_TE|CNTLA1_CKA1D|CNTLA1_MOD2;
	ld	a, #0x74
	out0	(_CNTLA1), a
;main.c:122: CNTLB1 = CNTLB1_SS0;
	ld	a, #0x01
	out0	(_CNTLB1), a
;main.c:125: (void)STAT1;
	in0	a, (_STAT1)
;main.c:126: }
	ret
;main.c:130: void delay_us(unsigned int us)
;	---------------------------------
; Function delay_us
; ---------------------------------
_delay_us::
;main.c:133: for (i = 0; i < us; i++)
	ld	bc, #0x0000
00103$:
	ld	a, c
	sub	a, l
	ld	a, b
	sbc	a, h
	ret	NC
;main.c:136: __asm__("nop");
	nop
;main.c:137: __asm__("nop");
	nop
;main.c:138: __asm__("nop");
	nop
;main.c:139: __asm__("nop");
	nop
;main.c:133: for (i = 0; i < us; i++)
	inc	bc
;main.c:141: }
	jr	00103$
;main.c:143: void delay_ms(unsigned int ms)
;	---------------------------------
; Function delay_ms
; ---------------------------------
_delay_ms::
;main.c:146: for (i = 0; i < ms; i++)
	ld	bc, #0x0000
00103$:
	ld	a, c
	sub	a, l
	ld	a, b
	sbc	a, h
	ret	NC
;main.c:148: delay_us(100);
	push	hl
	push	bc
	ld	hl, #0x0064
	call	_delay_us
	pop	bc
	pop	hl
;main.c:146: for (i = 0; i < ms; i++)
	inc	bc
;main.c:150: }
	jr	00103$
;main.c:153: void lcd_init(void)
;	---------------------------------
; Function lcd_init
; ---------------------------------
_lcd_init::
;main.c:155: delay_ms(40);
	ld	hl, #0x0028
	call	_delay_ms
;main.c:156: LCD_CMD = 0b00111000; // function set
	ld	a, #0x38
	out0	(_LCD_CMD), a
;main.c:157: delay_ms(15);
	ld	hl, #0x000f
	call	_delay_ms
;main.c:158: LCD_CMD = 0b00000110; // Entry mode set
	ld	a, #0x06
	out0	(_LCD_CMD), a
;main.c:159: delay_ms(1);
	ld	hl, #0x0001
	call	_delay_ms
;main.c:160: LCD_CMD = 0b00001111; // display on, cursor on, blink on
	ld	a, #0x0f
	out0	(_LCD_CMD), a
;main.c:161: delay_us(40);
	ld	hl, #0x0028
;main.c:162: }
	jp	_delay_us
;main.c:164: void lcd_clear(void)
;	---------------------------------
; Function lcd_clear
; ---------------------------------
_lcd_clear::
;main.c:166: LCD_CMD = 0x01;
	ld	a, #0x01
	out0	(_LCD_CMD), a
;main.c:167: delay_ms(15);
	ld	hl, #0x000f
;main.c:168: }
	jp	_delay_ms
;main.c:170: void lcd_setCursor(unsigned char row, unsigned char col)
;	---------------------------------
; Function lcd_setCursor
; ---------------------------------
_lcd_setCursor::
;main.c:172: LCD_CMD = 0x80 | (col + (row ? 0x40 : 0));
	or	a, a
	ld	a, #0x40
	jr	NZ, 00104$
	xor	a, a
00104$:
	add	a, l
	or	a, #0x80
	out0	(_LCD_CMD), a
;main.c:173: }
	ret
;main.c:175: void lcd_printChar(unsigned char b)
;	---------------------------------
; Function lcd_printChar
; ---------------------------------
_lcd_printChar::
	out0	(_LCD_DATA), a
;main.c:178: delay_us(40);
	ld	hl, #0x0028
;main.c:179: }
	jp	_delay_us
;main.c:181: void lcd_print(const char *s)
;	---------------------------------
; Function lcd_print
; ---------------------------------
_lcd_print::
;main.c:183: while(*s)
00101$:
	ld	a, (hl)
	or	a, a
	ret	Z
;main.c:185: lcd_printChar(*s++);
	inc	hl
	push	hl
	call	_lcd_printChar
	pop	hl
;main.c:187: }
	jr	00101$
;main.c:189: void lcd_put_hex1(const unsigned char n)
;	---------------------------------
; Function lcd_put_hex1
; ---------------------------------
_lcd_put_hex1::
;main.c:193: lcd_printChar(n + '0');
	ld	c, a
;main.c:191: if (n < 10)
	sub	a, #0x0a
	jr	NC, 00102$
;main.c:193: lcd_printChar(n + '0');
	ld	a, c
	add	a, #0x30
	jp	_lcd_printChar
00102$:
;main.c:197: lcd_printChar(n - 0x0a + 'A');
	ld	a, c
	add	a, #0x37
;main.c:199: }
	jp	_lcd_printChar
;main.c:201: void lcd_put_hex2(const unsigned char h)
;	---------------------------------
; Function lcd_put_hex2
; ---------------------------------
_lcd_put_hex2::
	ld	c, a
;main.c:203: lcd_put_hex1(h >> 4);
	ld	e, c
	ld	d, #16
	mlt	de
	push	bc
	ld	a, d
	call	_lcd_put_hex1
	pop	bc
;main.c:204: lcd_put_hex1(h & 0xf);
	ld	a, c
	and	a, #0x0f
;main.c:205: }
	jp	_lcd_put_hex1
;main.c:207: void main (void)
;	---------------------------------
; Function main
; ---------------------------------
_main::
;main.c:209: asci0_init();
	call	_asci0_init
;main.c:210: asci1_init();
	call	_asci1_init
;main.c:212: asci1_puts("\nFirmware built at " __DATE__ " " __TIME__ "\n");
	ld	hl, #___str_0
	call	_asci1_puts
;main.c:214: lcd_init();
	call	_lcd_init
;main.c:216: lcd_print("Cocus was here!!");
	ld	hl, #___str_1
	call	_lcd_print
;main.c:218: lcd_setCursor(1, 0);
	ld	l, #0x00
	ld	a, #0x01
	call	_lcd_setCursor
;main.c:219: lcd_print(__DATE__ " " __TIME__);
	ld	hl, #___str_2
	call	_lcd_print
;main.c:221: while (1)
00102$:
;main.c:223: CNTLA0 &= ~(CNTLA0_RTS0); // /RTS = 0, LED on
	in0	a, (_CNTLA0)
	and	a, #0xef
	out0	(_CNTLA0), a
;main.c:225: asci1_puts("Tick tick tick!\n");
	ld	hl, #___str_3
	call	_asci1_puts
;main.c:227: CNTLA0 |= CNTLA0_RTS0; // /RTS = 1, LED off
	in0	a, (_CNTLA0)
	or	a, #0x10
	out0	(_CNTLA0), a
;main.c:229: delay_ms(300);
	ld	hl, #0x012c
	call	_delay_ms
;main.c:231: }
	jr	00102$
___str_0:
	.db 0x0a
	.ascii "Firmware built at Nov 14 2025 15:07:25"
	.db 0x0a
	.db 0x00
___str_1:
	.ascii "Cocus was here!!"
	.db 0x00
___str_2:
	.ascii "Nov 14 2025 15:07:25"
	.db 0x00
___str_3:
	.ascii "Tick tick tick!"
	.db 0x0a
	.db 0x00
	.area _CODE
	.area _INITIALIZER
	.area _CABS (ABS)
