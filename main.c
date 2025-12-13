#include <stdint.h>
#include <stdio.h>

#include <z180.h>
#include "z180_internal.h"

/* Scheduler include files. */
#include <FreeRTOS.h>
#include <task.h>

/* pff */
#include <pff.h>

/**
 * There's a 16C2550, both chip selects are used; however, the register mapping is a little
 * bit odd. Register 0 for UART A is at 0x80, register 1 at 0x88, register 2 at 0x90, 3 at 0x98,
 * 4 at 0xa0, 5 at 0xa8, 6 at 0xb0 and 7 at 0xb8. For UART B, the IO addresses are 0x81/89/91/99/a1/a9/b1/b9.
 * NOTE: No IRQ :(
 */

// 0x80/88/90/98/a0/a8/b0/b8 -> UART_A
__sfr __at(0x80) EXT_UARTA_RBR_THR_DLL; /* Receiver buffer (read), transmitter holding register (write) / Divisor latch (LSB) */
__sfr __at(0x88) EXT_UARTA_IER_DLM;     /* Interrupt enable register / Divisor latch (MSB) */
__sfr __at(0x90) EXT_UARTA_IIR_FCR;     /* Interrupt identification register (read only) / FIFO control register (write) */
__sfr __at(0x98) EXT_UARTA_LCR;         /* Line control register */
__sfr __at(0xa0) EXT_UARTA_MCR;         /* Modem control register */
__sfr __at(0xa8) EXT_UARTA_LSR;         /* Line status register */
__sfr __at(0xb0) EXT_UARTA_MSR;         /* Modem status register */
__sfr __at(0xb8) EXT_UARTA_SCR;         /* Scratch register */

// 0x81/89/91/99/a1/a9/b1/b9 -> UART_B
__sfr __at(0x81) EXT_UARTB_RBR_THR_DLL; /* Receiver buffer (read), transmitter holding register (write) / Divisor latch (LSB) */
__sfr __at(0x89) EXT_UARTB_IER_DLM;     /* Interrupt enable register / Divisor latch (MSB) */
__sfr __at(0x91) EXT_UARTB_IIR_FCR;     /* Interrupt identification register (read only) / FIFO control register (write) */
__sfr __at(0x99) EXT_UARTB_LCR;         /* Line control register */
__sfr __at(0xa1) EXT_UARTB_MCR;         /* Modem control register */
__sfr __at(0xa9) EXT_UARTB_LSR;         /* Line status register */
__sfr __at(0xb1) EXT_UARTB_MSR;         /* Modem status register */
__sfr __at(0xb9) EXT_UARTB_SCR;         /* Scratch register */

/**
 * Standard HD44780 LCD at IO port 0x40 and 0x41. R/S connected directly to Z180's A0.
 */
__sfr __at(0x40) LCD_CMD;  /* */
__sfr __at(0x41) LCD_DATA; /* */

void asci0_putc(char c)
{
    while ((STAT0 & STAT0_TDRE) == 0)
        ; // wait for TX buffer empty
    TDR0 = c;
}

char asci0_getc(void)
{
    while ((STAT0 & STAT0_RDRF) == 0)
        ; // wait for RX data ready
    return RDR0;
}

void asci0_init(void)
{
    // RE : Receiver Enable
    // TE : Transmitter Enable
    // RTS: /RTS0 = 1
    // MOD2 : 8 bits data (No parity, 1 stop bit)
    CNTLA0 = CNTLA0_RE | CNTLA0_RTS0 | CNTLA0_TE | CNTLA0_MOD2;

    // DR=0 -> sampling rate = 16
    // PS=0 -> prescaler = /10
    // SS=001 -> /2 ratio
    // General divide ratio = phi/320
    CNTLB0 = CNTLB0_SS0;

    // Clear status (read)
    (void)STAT0;
}

/* for sdcc's printf */
int putchar(int c)
{
    while ((STAT1 & STAT1_TDRE) == 0)
        ; // wait for TX buffer empty
    TDR1 = c;
    return c;
}

int getchar(void)
{
    while ((STAT1 & STAT1_RDRF) == 0)
        ; // wait for RX data ready
    return RDR1;
}

void asci1_init(void)
{
    // RE : Receiver Enable
    // TE : Transmitter Enable
    // CKA1D : CKA1 is disabled (pin used as /TEND)
    // MOD2 : 8 bits data (No parity, 1 stop bit)
    CNTLA1 = CNTLA1_RE | CNTLA1_TE | CNTLA1_CKA1D | CNTLA1_MOD2;

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
    vTaskDelay(pdMS_TO_TICKS(40));
    LCD_CMD = 0b00111000; // function set
    vTaskDelay(pdMS_TO_TICKS(15));
    LCD_CMD = 0b00000110; // Entry mode set
    vTaskDelay(pdMS_TO_TICKS(4));
    LCD_CMD = 0b00001111; // display on, cursor on, blink on
    vTaskDelay(pdMS_TO_TICKS(40));
}

void lcd_clear(void)
{
    LCD_CMD = 0x01;
    vTaskDelay(pdMS_TO_TICKS(15));
}

void lcd_setCursor(unsigned char row, unsigned char col)
{
    LCD_CMD = 0x80 | (col + (row ? 0x40 : 0));
}

void lcd_printChar(unsigned char b)
{
    LCD_DATA = b;
    vTaskDelay(pdMS_TO_TICKS(1));
}

void lcd_print(const char *s)
{
    while (*s)
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

/*-----------------------------------------------------------*/
static void TaskBlinkGreenLED(void *pvParameters)
{
    (void)pvParameters;

    TickType_t xLastWakeTime;
    /* The xLastWakeTime variable needs to be initialised with the current tick
    count.  Note that this is the only time we access this variable.  From this
    point on xLastWakeTime is managed automatically by the xTaskDelayUntil()
    API function. */
    xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        CNTLA0 &= ~(CNTLA0_RTS0); // /RTS0 = 0, LED on

        xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(75));

        // printf("Tick tick tick %d!\n", xLastWakeTime);
        CNTLA0 |= CNTLA0_RTS0; // /RTS0 = 1, LED off

        xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(75));

        // printf("xTaskGetTickCount %u\r\n", xTaskGetTickCount());
        // printf("GreenLED HighWater @ %u\r\n", uxTaskGetStackHighWaterMark(NULL));
    }
}

static void TaskUartServer(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        // use IRQ, not polling!
        xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50));

        if ((STAT1 & STAT1_RDRF) == 0)
        {
            continue;
        }

        // echo it back
        printf("%c", getchar());
    }
}

static void TaskLcd(void *pvParameters)
{
    (void)pvParameters;
    lcd_init();
    lcd_print("Cocus was here!!");
    lcd_setCursor(1, 0);
    lcd_print(__DATE__ " " __TIME__);

    vTaskSuspend(NULL);
    vTaskDelete(NULL);
}

FATFS FatFs; /* FatFs work area */
DIR Dir;     /* Directory object */
FILINFO Finfo;

static void TaskSD(void *pvParameters)
{
    (void)pvParameters;
    FRESULT fr;

    fr = pf_mount(&FatFs);
    printf("Mount result = %d\n", fr);

    fr = pf_opendir(&Dir, "");
    if (fr)
    {
        printf("Opendir result = %d\n", fr);
        vTaskSuspend(NULL);
    }

    for (;;)
    {
        fr = pf_readdir(&Dir, &Finfo); /* Read a directory item */
        if (fr || !Finfo.fname[0])
            break; /* Error or end of dir */
        if (Finfo.fattrib & AM_DIR)
            printf("   <dir>  %s\n", Finfo.fname);
        else
            printf("%8lu  %s\n", Finfo.fsize, Finfo.fname);
    }

    vTaskSuspend(NULL);
}

void int_init(void)
{
    /* Table starts at 0xffe0 */
    __asm__("ld a, #0xff    \n"
            "ld i, a        \n");
    IL = 0xe0;
}

static const struct
{
    TaskFunction_t pxTaskCode;
    const char *const pcName;
    const uint16_t uxStackDepth;
    void *const pvParameters;
    UBaseType_t uxPriority;
    TaskHandle_t *const pxCreatedTask;
} tasks[] =
{
    {TaskLcd, "LCD", 128, NULL, 1, NULL},
    {TaskBlinkGreenLED, "GreenLED", 128, NULL, 2, NULL},
    {TaskUartServer, "UARTServer", 128, NULL, 1, NULL},
    {TaskSD, "SD", 128, NULL, 1, NULL}
};
#define NUM_TASKS (sizeof(tasks)/sizeof(tasks[0]))

void main(void)
{
    /* Setup the interrupt vector table addresses in RAM */
    int_init();

    asci0_init();
    asci1_init();

    printf("\nFirmware built at " __DATE__ " " __TIME__ "\n");
    printf("CPU phi = " string(__CPU_CLOCK) " Hz\n");

    /* I've connected an LED from Vcc to /RTS0 (free GPIO!) */
    CNTLA0 &= ~(CNTLA0_RTS0); // /RTS0 = 0, LED on

    printf("Creating tasks:\n");

    for (int i = 0; i < NUM_TASKS; i++)
    {
        BaseType_t res = xTaskCreate(
            tasks[i].pxTaskCode,
            tasks[i].pcName,
            tasks[i].uxStackDepth,
            tasks[i].pvParameters,
            tasks[i].uxPriority,
            tasks[i].pxCreatedTask
        );
        printf("Task '%s' created, ret = 0x%.2x\n", tasks[i].pcName, res);
    }

    printf("Jumping in\n");
    vTaskStartScheduler();

    while (1)
    {
        printf("Shouldn't be here!\n");
    }
}
