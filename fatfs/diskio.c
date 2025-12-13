#include "ff.h"     /* Basic definitions of FatFs */
#include "diskio.h" /* Declarations FatFs MAI */

#include <stdio.h>
#include <z180.h>
#include "../z180_internal.h"

/* Port controls (Platform dependent) */
__sfr __at(0x4a) EXT_CS; /* this is a 737 buffer, whose Dx pins are connected to the data bus, and Q3 is connected to the CS of the SD card */

#define CS_LOW() EXT_CS = 0           /* MMC_CS = low */
#define CS_HIGH() EXT_CS = 0b00001000 /* MMC_CS = high */

#define FCLK_LOW() CNTR = CNTR_SS2 | CNTR_SS1 /* CKS baud = phi ÷ 1280 */
#define FCLK_FAST() CNTR = 0                  /* CKS baud = phi ÷ 20 */

/* Definitions for MMC/SDC command */
#define CMD0 (0)           /* GO_IDLE_STATE */
#define CMD1 (1)           /* SEND_OP_COND (MMC) */
#define ACMD41 (0x80 + 41) /* SEND_OP_COND (SDC) */
#define CMD8 (8)           /* SEND_IF_COND */
#define CMD9 (9)           /* SEND_CSD */
#define CMD10 (10)         /* SEND_CID */
#define CMD12 (12)         /* STOP_TRANSMISSION */
#define ACMD13 (0x80 + 13) /* SD_STATUS (SDC) */
#define CMD16 (16)         /* SET_BLOCKLEN */
#define CMD17 (17)         /* READ_SINGLE_BLOCK */
#define CMD18 (18)         /* READ_MULTIPLE_BLOCK */
#define CMD23 (23)         /* SET_BLOCK_COUNT (MMC) */
#define ACMD23 (0x80 + 23) /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24 (24)         /* WRITE_BLOCK */
#define CMD25 (25)         /* WRITE_MULTIPLE_BLOCK */
#define CMD32 (32)         /* ERASE_ER_BLK_START */
#define CMD33 (33)         /* ERASE_ER_BLK_END */
#define CMD38 (38)         /* ERASE */
#define CMD48 (48)         /* READ_EXTR_SINGLE */
#define CMD49 (49)         /* WRITE_EXTR_SINGLE */
#define CMD55 (55)         /* APP_CMD */
#define CMD58 (58)         /* READ_OCR */

static volatile DSTATUS Stat = STA_NOINIT; /* Physical drive status */
static BYTE CardType;                      /* Card type flags */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
    BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
    return Stat;
}

/*-----------------------------------------------------------------------*/
/* Transmit/Receive data from/to MMC via SPI  (Platform dependent)       */
/*-----------------------------------------------------------------------*/

/* Initialize MMC interface */
static void init_spi(void)
{
    FCLK_LOW();
    CS_HIGH(); /* Set CS# high */

    uint16_t Timer1 = 1000;
    for (; Timer1; Timer1--)
        ; /* 10ms */
}

/*-----------------------------------------------------------------------*/
/* Transmit bytes to the card (bitbanging)                               */
/*-----------------------------------------------------------------------*/

static void xmit_mmc(
    const BYTE *buff, /* Data to be sent */
    UINT bc           /* Number of bytes to send */
)
{
    BYTE d;

    do
    {
        d = *buff++; /* Get a byte to be sent */

        /* Sadly the CSI uses LSB first, so swap it */
        d = ((d & 0b00000001) ? 0b10000000 : 0) |
            ((d & 0b00000010) ? 0b01000000 : 0) |
            ((d & 0b00000100) ? 0b00100000 : 0) |
            ((d & 0b00001000) ? 0b00010000 : 0) |
            ((d & 0b00010000) ? 0b00001000 : 0) |
            ((d & 0b00100000) ? 0b00000100 : 0) |
            ((d & 0b01000000) ? 0b00000010 : 0) |
            ((d & 0b10000000) ? 0b00000001 : 0);

        /* b. Write the transmit data into TRDR. */
        TRDR = d;
        /* c. Set the TE bit in CNTR to 1. */
        CNTR |= CNTR_TE;
        /* a. Poll the TE bit in CNTR until TE = 0. */
        while ((CNTR & CNTR_TE))
            ;

    } while (--bc);
}

/*-----------------------------------------------------------------------*/
/* Receive bytes from the card (bitbanging)                              */
/*-----------------------------------------------------------------------*/

static void rcvr_mmc(
    BYTE *buff, /* Pointer to read buffer */
    UINT bc     /* Number of bytes to receive */
)
{
    BYTE r;

    TRDR = 0xff; /* Send 0xFF */
    do
    {
        /* a. Poll the RE bit in CNTR until RE = 0.*/
        while ((CNTR & CNTR_RE))
            ;
        /* b. Set the RE bit in CNTR to 1. */
        CNTR |= CNTR_RE;
        /* c. Poll the RE bit in CNTR until RE = 0. */
        while ((CNTR & CNTR_RE))
            ;
        /* d. Read the receive data from TRDR. */
        r = TRDR;

        /* Sadly the CSI uses LSB first, so swap it */
        r = ((r & 0b00000001) ? 0b10000000 : 0) |
            ((r & 0b00000010) ? 0b01000000 : 0) |
            ((r & 0b00000100) ? 0b00100000 : 0) |
            ((r & 0b00001000) ? 0b00010000 : 0) |
            ((r & 0b00010000) ? 0b00001000 : 0) |
            ((r & 0b00100000) ? 0b00000100 : 0) |
            ((r & 0b01000000) ? 0b00000010 : 0) |
            ((r & 0b10000000) ? 0b00000001 : 0);

        /* Store a received byte */
        *buff++ = r;
    } while (--bc);
}

/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

static int wait_ready(        /* 1:Ready, 0:Timeout */
                      UINT wt /* Timeout [ms] */
)
{
    BYTE d;

    uint32_t Timer2 = wt * 1000;
    do
    {
        rcvr_mmc(&d, 1);
        /* This loop takes a time. Insert rot_rdq() here for multitask envilonment. */
        Timer2--;
    } while (d != 0xFF && Timer2); /* Wait for card goes ready or timeout */

    return (d == 0xFF) ? 1 : 0;
}

/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

static void deselect(void)
{
    BYTE d;

    CS_HIGH();       /* Set CS# high */
    rcvr_mmc(&d, 1); /* Dummy clock (force DO hi-z for multiple slave SPI) */
}

/*-----------------------------------------------------------------------*/
/* Select the card and wait for ready                                    */
/*-----------------------------------------------------------------------*/

static int select(void) /* 1:Successful, 0:Timeout */
{
    BYTE d;

    CS_LOW();        /* Set CS# low */
    rcvr_mmc(&d, 1); /* Dummy clock (force DO enabled) */
    if (wait_ready(500))
        return 1; /* Wait for card ready */

    deselect();
    return 0; /* Failed */
}

/*-----------------------------------------------------------------------*/
/* Receive a data packet from the card                                   */
/*-----------------------------------------------------------------------*/

static int rcvr_datablock(            /* 1:OK, 0:Failed */
                          BYTE *buff, /* Data buffer to store received data */
                          UINT btr    /* Byte count */
)
{
    BYTE d[2];
    UINT tmr;

    for (tmr = 1000; tmr; tmr--)
    { /* Wait for data packet in timeout of 100ms */
        rcvr_mmc(d, 1);
        if (d[0] != 0xFF)
            break;
        // dly_us(100);
    }
    if (d[0] != 0xFE)
        return 0; /* If not valid data token, return with error */

    rcvr_mmc(buff, btr); /* Receive the data block into buffer */
    rcvr_mmc(d, 2);      /* Discard CRC */

    return 1; /* Return with success */
}

/*-----------------------------------------------------------------------*/
/* Send a data packet to the card                                        */
/*-----------------------------------------------------------------------*/

static int xmit_datablock(                  /* 1:OK, 0:Failed */
                          const BYTE *buff, /* 512 byte data block to be transmitted */
                          BYTE token        /* Data/Stop token */
)
{
    BYTE d[2];

    if (!wait_ready(500))
        return 0;

    d[0] = token;
    xmit_mmc(d, 1); /* Xmit a token */
    if (token != 0xFD)
    {                              /* Is it data token? */
        xmit_mmc(buff, 512);       /* Xmit the 512 byte data block to MMC */
        rcvr_mmc(d, 2);            /* Xmit dummy CRC (0xFF,0xFF) */
        rcvr_mmc(d, 1);            /* Receive data response */
        if ((d[0] & 0x1F) != 0x05) /* If not accepted, return with error */
            return 0;
    }

    return 1;
}

/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/

static BYTE send_cmd(          /* Returns R1 resp (bit7==1:Send failed) */
                     BYTE cmd, /* Command index */
                     DWORD arg /* Argument */
)
{
    BYTE n, d, buf[6];

    if (cmd & 0x80)
    { /* ACMD<n> is the command sequense of CMD55-CMD<n> */
        cmd &= 0x7F;
        n = send_cmd(CMD55, 0);
        if (n > 1)
            return n;
    }

    /* Select the card and wait for ready except to stop multiple block read */
    if (cmd != CMD12)
    {
        deselect();
        if (!select())
            return 0xFF;
    }

    /* Send a command packet */
    buf[0] = 0x40 | cmd;        /* Start + Command index */
    buf[1] = (BYTE)(arg >> 24); /* Argument[31..24] */
    buf[2] = (BYTE)(arg >> 16); /* Argument[23..16] */
    buf[3] = (BYTE)(arg >> 8);  /* Argument[15..8] */
    buf[4] = (BYTE)arg;         /* Argument[7..0] */
    n = 0x01;                   /* Dummy CRC + Stop */
    if (cmd == CMD0)
        n = 0x95; /* (valid CRC for CMD0(0)) */
    if (cmd == CMD8)
        n = 0x87; /* (valid CRC for CMD8(0x1AA)) */
    buf[5] = n;
    xmit_mmc(buf, 6);

    /* Receive command response */
    if (cmd == CMD12)
        rcvr_mmc(&d, 1); /* Skip a stuff byte when stop reading */
    n = 10;              /* Wait for a valid response in timeout of 10 attempts */
    do
        rcvr_mmc(&d, 1);
    while ((d & 0x80) && --n);

    return d; /* Return with the response value */
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
    BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
    BYTE n, cmd, ty, ocr[4];
    uint16_t Timer1 = 0xFFFF;

    if (pdrv)
        return STA_NOINIT; /* Supports only drive 0 */
    init_spi();            /* Initialize SPI */

    for (n = 10; n; n--)
        rcvr_mmc(ocr, 1); /* Apply 80 dummy clocks and the card gets ready to receive command */

    ty = 0;
    if (send_cmd(CMD0, 0) == 1)
    {                    /* Put the card SPI mode */
        Timer1 = 0xFFFF; /* Initialization timeout of 1000 msec */
        if (send_cmd(CMD8, 0x1AA) == 1)
        {                     /* Is the card SDv2? */
            rcvr_mmc(ocr, 4); /* Get trailing return value of R7 resp */
            if (ocr[2] == 0x01 && ocr[3] == 0xAA)
            { /* The card can work at vdd range of 2.7-3.6V */
                while (Timer1 && send_cmd(ACMD41, 1UL << 30))
                    ; /* Wait for leaving idle state (ACMD41 with HCS bit) */
                if (Timer1 && send_cmd(CMD58, 0) == 0)
                { /* Check CCS bit in the OCR */
                    rcvr_mmc(ocr, 4);
                    ty = (ocr[0] & 0x40) ? CT_SDC2 | CT_BLOCK : CT_SDC2; /* Check if the card is SDv2 */
                }
            }
        }
        else
        { /* SDv1 or MMCv3 */
            if (send_cmd(ACMD41, 0) <= 1)
            {
                ty = CT_SDC1;
                cmd = ACMD41; /* SDv1 */
            }
            else
            {
                ty = CT_MMC3;
                cmd = CMD1; /* MMCv3 */
            }
            while (Timer1 && send_cmd(cmd, 0))
                ; /* Wait for leaving idle state */
            if (!Timer1 || send_cmd(CMD16, 512) != 0)
            { /* Set R/W block length to 512 */
                ty = 0;
            }
        }
    }
    CardType = ty;
    deselect();

    if (ty)
    {                        /* Initialization succeded */
        Stat &= ~STA_NOINIT; /* Clear STA_NOINIT */
        FCLK_FAST();
        printf("Init ok, card type is %d\n", CardType);
    }
    else
    {
        /* Initialization failed */
        printf("Init failed!\n");
    }

    return Stat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
    BYTE pdrv,    /* Physical drive nmuber to identify the drive */
    BYTE *buff,   /* Data buffer to store read data */
    LBA_t sector, /* Start sector in LBA */
    UINT count    /* Number of sectors to read */
)
{
    BYTE cmd;
    DWORD sect = (DWORD)sector;

    if (disk_status(pdrv) & STA_NOINIT)
        return RES_NOTRDY;
    if (!(CardType & CT_BLOCK))
        sect *= 512; /* Convert LBA to byte address if needed */

    cmd = count > 1 ? CMD18 : CMD17; /*  READ_MULTIPLE_BLOCK : READ_SINGLE_BLOCK */
    if (send_cmd(cmd, sect) == 0)
    {
        do
        {
            if (!rcvr_datablock(buff, 512))
                break;
            buff += 512;
        } while (--count);
        if (cmd == CMD18)
            send_cmd(CMD12, 0); /* STOP_TRANSMISSION */
    }
    deselect();

    return count ? RES_ERROR : RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(
    BYTE pdrv,        /* Physical drive nmuber to identify the drive */
    const BYTE *buff, /* Data to be written */
    LBA_t sector,     /* Start sector in LBA */
    UINT count        /* Number of sectors to write */
)
{
    DWORD sect = (DWORD)sector;

    if (disk_status(pdrv) & STA_NOINIT)
        return RES_NOTRDY;
    if (!(CardType & CT_BLOCK))
        sect *= 512; /* Convert LBA to byte address if needed */

    if (count == 1)
    {                                    /* Single block write */
        if ((send_cmd(CMD24, sect) == 0) /* WRITE_BLOCK */
            && xmit_datablock(buff, 0xFE))
            count = 0;
    }
    else
    { /* Multiple block write */
        if (CardType & CT_SDC)
            send_cmd(ACMD23, count);
        if (send_cmd(CMD25, sect) == 0)
        { /* WRITE_MULTIPLE_BLOCK */
            do
            {
                if (!xmit_datablock(buff, 0xFC))
                    break;
                buff += 512;
            } while (--count);
            if (!xmit_datablock(0, 0xFD)) /* STOP_TRAN token */
                count = 1;
        }
    }
    deselect();

    return count ? RES_ERROR : RES_OK;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(
    BYTE pdrv, /* Physical drive nmuber (0..) */
    BYTE cmd,  /* Control code */
    void *buff /* Buffer to send/receive control data */
)
{
    DRESULT res;
    BYTE n, csd[16];
    DWORD cs;

    if (disk_status(pdrv) & STA_NOINIT)
        return RES_NOTRDY; /* Check if card is in the socket */

    res = RES_ERROR;
    switch (cmd)
    {
    case CTRL_SYNC: /* Make sure that no pending write process */
        if (select())
            res = RES_OK;
        break;

    case GET_SECTOR_COUNT: /* Get number of sectors on the disk (DWORD) */
        if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16))
        {
            if ((csd[0] >> 6) == 1)
            { /* SDC ver 2.00 */
                cs = csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1;
                *(LBA_t *)buff = cs << 10;
            }
            else
            { /* SDC ver 1.XX or MMC */
                n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
                cs = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
                *(LBA_t *)buff = cs << (n - 9);
            }
            res = RES_OK;
        }
        break;

    case GET_BLOCK_SIZE: /* Get erase block size in unit of sector (DWORD) */
        *(DWORD *)buff = 128;
        res = RES_OK;
        break;

    default:
        res = RES_PARERR;
    }

    deselect();

    return res;
}
