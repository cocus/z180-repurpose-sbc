#pragma once

// ASCI Control Register A 0 (CNTLA0: 00H)
#define CNTLA0_MPE     0x80
#define CNTLA0_RE      0x40
#define CNTLA0_TE      0x20
#define CNTLA0_RTS0    0x10
#define CNTLA0_EFR     0x08
#define CNTLA0_MOD2    0x04
#define CNTLA0_MOD1    0x02
#define CNTLA0_MOD0    0x01

// ASCI Control Register A 1 (CNTLA1: 01H)
#define CNTLA1_MPE     0x80
#define CNTLA1_RE      0x40
#define CNTLA1_TE      0x20
#define CNTLA1_CKA1D   0x10
#define CNTLA1_EFR     0x08
#define CNTLA1_MOD2    0x04
#define CNTLA1_MOD1    0x02
#define CNTLA1_MOD0    0x01

// ASCI Control Register B 0 (CNTLB0: 02H)
#define CNTLB0_MPBT    0x80
#define CNTLB0_MP      0x40
#define CNTLB0_CTS     0x20
#define CNTLB0_PEO     0x10
#define CNTLB0_DR      0x08
#define CNTLB0_SS2     0x04
#define CNTLB0_SS1     0x02
#define CNTLB0_SS0     0x01

// ASCI Control Register B 1 (CNTLB1: 03H)
#define CNTLB1_MPBT    0x80
#define CNTLB1_MP      0x40
#define CNTLB1_CTS     0x20
#define CNTLB1_PEO     0x10
#define CNTLB1_DR      0x08
#define CNTLB1_SS2     0x04
#define CNTLB1_SS1     0x02
#define CNTLB1_SS0     0x01

// ASCI Status Register 0 (STAT0: 04H)
#define STAT0_RDRF     0x80
#define STAT0_OVRN     0x40
#define STAT0_PE       0x20
#define STAT0_FE       0x10
#define STAT0_RIE      0x08
#define STAT0_DCD0     0x04
#define STAT0_TDRE     0x02
#define STAT0_TIE      0x01

// ASCI Status Register 1 (STAT1: 05H)
#define STAT1_RDRF     0x80
#define STAT1_OVRN     0x40
#define STAT1_PE       0x20
#define STAT1_FE       0x10
#define STAT1_RIE      0x08
#define STAT1_CTS1E    0x04
#define STAT1_TDRE     0x02
#define STAT1_TIE      0x01

// CSI/0
// ------------------------------------

// CSI/O Control Register (CNTR: 0AH)
#define CNTR_EF        0x80
#define CNTR_EIE       0x40
#define CNTR_RE        0x20
#define CNTR_TE        0x10
#define CNTR_SS2       0x04
#define CNTR_SS1       0x02
#define CNTR_SS0       0x01

// Timer
// ------------------------------------

// Timer Control Register (TCR: 10H)
#define TCR_TIF1        0x80
#define TCR_TIF0        0x40
#define TCR_TIE1        0x20
#define TCR_TIE0        0x10
#define TCR_TOC1       0x08
#define TCR_TOC0       0x04
#define TCR_TDE1       0x02
#define TCR_TDE0       0x01

// DMA
// ---------------------------------------------------------------------------


// DMA/WAIT Control Register (DCNTL: 32H)

// INT
// ---------------------------------------------------------------------------

// INT/TRAP Control Register (ITC: 34H)
#define ITC_TRAP        0x80
#define ITC_UFO         0x40
#define ITC_ITE2        0x04
#define ITC_ITE1        0x02
#define ITC_ITE0        0x01


// Refresh
// ---------------------------------------------------------------------------

// Refresh Control Register (RCR: 36H)
#define RCR_REFE       0x80
#define RCR_REFW       0x40
#define RCR_CYC1       0x02
#define RCR_CYC0       0x01

// I/O
// ---------------------------------------------------------------------------

// Operation Mode Control Register (OMCR: 3EH)
#define OMCR_M1E       0x80
#define OMCR_M1TE      0x40
#define OMCR_IOC       0x20

// I/O Control Register (ICR: 3FH)
#define ICR_IOA7       0x80
#define ICR_IOA6       0x40
#define ICR_IOSTP      0x20
