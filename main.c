#include <z180.h>
#include <stdint.h>
#include "z180_internal.h"
#include <stdio.h>

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

/* for sdcc's printf */
int putchar (int c)
{
    while ((STAT1 & STAT1_TDRE) == 0) ; // wait for TX buffer empty
    TDR1 = c;
    return c;
}

int getchar(void)
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


/* Scheduler include files. */
#include <FreeRTOS.h>
#include <timers.h>

/*-----------------------------------------------------------*/
static void TaskBlinkGreenLED(void *pvParameters)
{
    (void) pvParameters;

    TickType_t xLastWakeTime;
    /* The xLastWakeTime variable needs to be initialised with the current tick
    count.  Note that this is the only time we access this variable.  From this
    point on xLastWakeTime is managed automatically by the xTaskDelayUntil()
    API function. */
    xLastWakeTime = xTaskGetTickCount();

    printf("Hi from task TaskBlinkGreenLED\n");


    for(;;)
    {
        CNTLA0 &= ~(CNTLA0_RTS0); // /RTS0 = 0, LED on

        xTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 75 ) );

        printf("Tick tick tick %d!\n", xLastWakeTime);
        CNTLA0 |= CNTLA0_RTS0; // /RTS0 = 1, LED off

        xTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 75 ) );

        //printf("xTaskGetTickCount %u\r\n", xTaskGetTickCount());
        //printf("GreenLED HighWater @ %u\r\n", uxTaskGetStackHighWaterMark(NULL));
    }
}

void int_init(void)
{
    /* Table starts at 0xffe0 */
    __asm__("ld a, #0xff    \n"\
            "ld i, a        \n");
    IL = 0xe0;
}

void main (void)
{
    asci0_init();
    asci1_init();

    printf("\nFirmware built at " __DATE__ " " __TIME__ "\n");

    lcd_init();

    lcd_print("Cocus was here!!");

    lcd_setCursor(1, 0);
    lcd_print(__DATE__ " " __TIME__);

    /* Setup the interrupt vector table addresses in RAM */
    int_init();

    /* I've connected an LED from Vcc to /RTS0 (free GPIO!) */
    CNTLA0 &= ~(CNTLA0_RTS0); // /RTS0 = 0, LED on


    printf("Creating task\n");
    BaseType_t res = xTaskCreate(
        TaskBlinkGreenLED
        ,  "GreenLED"
        ,  128
        ,  NULL
        ,  2
        ,  NULL ); //
    printf("Created, ret = 0x%.2x\n", res);
    printf("Jumping in\n");
    vTaskStartScheduler();

    while (1)
    {
        printf("Shouldn't be here!\n");
        delay_ms(300);
    }
}


