#include <z180.h>
#include <stdint.h>
#include "z180_internal.h"


/**
 * Standard HD44780 LCD at IO port 0x40 and 0x41. R/S connected directly to Z180's A0.
 */
__sfr __at (0x40) LCD_CMD;   /* */
__sfr __at (0x41) LCD_DATA;  /* */


void asci0_putc(char c)
{
    while ((STAT0 & STAT0_TDRE) == 0) ; // wait for TX buffer empty
    TDR0 = c;
}

void asci0_puts(const char* s)
{
    while(*s)
    {
        asci0_putc(*s++);
    }
}

void asci0_put_hex1(const unsigned char n)
{
    if (n < 10)
    {
        asci0_putc(n + '0');
    }
    else
    {
        asci0_putc(n + 'A' - 10);
    }
}

void asci0_put_hex2(const unsigned char h)
{
    asci0_put_hex1(h >> 4);
    asci0_put_hex1(h & 0xf);
}

char asci0_getc(void)
{
    while ((STAT0 & STAT0_RDRF) == 0) ; // wait for RX data ready
    return RDR0;
}

void asci0_init(void)
{
    // RE : Receiver Enable
    // TE : Transmitter Enable
    // RTS: /RTS0 = 1
    // MOD2 : 8 bits data (No parity, 1 stop bit)
    CNTLA0 = CNTLA0_RE|CNTLA0_RTS0|CNTLA0_TE|CNTLA0_MOD2;

    // DR=0 -> sampling rate = 16
    // PS=0 -> prescaler = /10
    // SS=001 -> /2 ratio
    // General divide ratio = phi/320
    CNTLB0 = CNTLB0_SS0;

    // Clear status (read)
    (void)STAT0;
}




void asci1_putc(char c)
{
    while ((STAT1 & STAT1_TDRE) == 0) ; // wait for TX buffer empty
    TDR1 = c;
}

void asci1_puts(const char* s)
{
    while(*s)
    {
        asci1_putc(*s++);
    }
}

void asci1_put_hex1(const unsigned char n)
{
    if (n < 10)
    {
        asci1_putc(n + '0');
    }
    else
    {
        asci1_putc(n + 'A' - 10);
    }
}

void asci1_put_hex2(const unsigned char h)
{
    asci1_put_hex1(h >> 4);
    asci1_put_hex1(h & 0xf);
}

char asci1_getc(void)
{
    while ((STAT1 & STAT1_RDRF) == 0) ; // wait for RX data ready
    return RDR1;
}

void asci1_init(void)
{
    // RE : Receiver Enable
    // TE : Transmitter Enable
    // CKA1D : CKA1 is disabled (pin used as /TEND)
    // MOD2 : 8 bits data (No parity, 1 stop bit)
    CNTLA1 = CNTLA1_RE|CNTLA1_TE|CNTLA1_CKA1D|CNTLA1_MOD2;

    // DR=0 -> sampling rate = 16
    // PS=0 -> prescaler = /10
    // SS=001 -> /2 ratio
    // General divide ratio = phi/320
    CNTLB1 = CNTLB1_SS0;

    // Clear status (read)
    (void)STAT1;
}



void delay_us(unsigned int us)
{
    unsigned int i;
    for (i = 0; i < us; i++)
    {
        /* This is not accurate */
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
        __asm__("nop");
    }
}

void delay_ms(unsigned int ms)
{
    unsigned int i;
    for (i = 0; i < ms; i++)
    {
        delay_us(100);
    }
}


void lcd_init(void)
{
    delay_ms(40);
    LCD_CMD = 0b00111000; // function set
    delay_ms(15);
    LCD_CMD = 0b00000110; // Entry mode set
    delay_ms(1);
    LCD_CMD = 0b00001111; // display on, cursor on, blink on
    delay_us(40);
}

void lcd_clear(void)
{
    LCD_CMD = 0x01;
    delay_ms(15);
}

void lcd_setCursor(unsigned char row, unsigned char col)
{
    LCD_CMD = 0x80 | (col + (row ? 0x40 : 0));
}

void lcd_printChar(unsigned char b)
{
    LCD_DATA = b;
    delay_us(40);
}

void lcd_print(const char *s)
{
    while(*s)
    {
        lcd_printChar(*s++);
    }
}

void lcd_put_hex1(const unsigned char n)
{
    if (n < 10)
    {
        lcd_printChar(n + '0');
    }
    else
    {
        lcd_printChar(n - 0x0a + 'A');
    }
}

void lcd_put_hex2(const unsigned char h)
{
    lcd_put_hex1(h >> 4);
    lcd_put_hex1(h & 0xf);
}

void main (void)
{
    asci0_init();
    asci1_init();

    asci1_puts("\nFirmware built at " __DATE__ " " __TIME__ "\n");

    lcd_init();

    lcd_print("Cocus was here!!");

    lcd_setCursor(1, 0);
    lcd_print(__DATE__ " " __TIME__);

    while (1)
    {
        CNTLA0 &= ~(CNTLA0_RTS0); // /RTS = 0, LED on

        asci1_puts("Tick tick tick!\n");

        CNTLA0 |= CNTLA0_RTS0; // /RTS = 1, LED off

        delay_ms(300);
    }
}


