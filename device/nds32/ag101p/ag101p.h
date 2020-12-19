/*
 * Copyright (c) 2012-2018 Andes Technology Corporation
 * All rights reserved.
 *
 */

#ifndef __AG101P_H__
#define __AG101P_H__

#ifndef __ASSEMBLER__
#include <inttypes.h>
#include <nds32_intrinsic.h>
#endif

/*****************************************************************************
 * System clock
 ****************************************************************************/
#define KHz                     1000
#define MHz                     1000000

#define OSCFREQ                 (20 * MHz)
#ifdef CFG_16MB
#define CPUFREQ                 (30 * MHz)
#define HCLKFREQ                (CPUFREQ / 1)
#define PCLKFREQ                (CPUFREQ / 2)
#define UCLKFREQ                (PCLKFREQ)
#else
#define CPUFREQ                 (60 * MHz)
#define HCLKFREQ                (CPUFREQ / 2)
#define PCLKFREQ                (CPUFREQ / 4)
#define UCLKFREQ                (PCLKFREQ)
#endif

/*****************************************************************************
 * IRQ Vector
 ****************************************************************************/
#define IRQ_AHBC_VECTOR         0
#define IRQ_RESERVE1_VECTOR     1
#define IRQ_SSP1_VECTOR         2
#define IRQ_I2C_VECTOR          3
#define IRQ_LDMA_VECTOR         4
#define IRQ_SDC_VECTOR          5
#define IRQ_SSP2_VECTOR         6
#define IRQ_UART2_VECTOR        7
#define IRQ_PMU_VECTOR          8
#define IRQ_SWI_VECTOR          9
#define IRQ_PFM_VECTOR          10
#define IRQ_UART1_VECTOR        11
#define IRQ_RESERVE12_VECTOR    12
#define IRQ_GPIO_VECTOR         13
#define IRQ_TIMER2_VECTOR       14
#define IRQ_TIMER3_VECTOR       15
#define IRQ_WDT_VECTOR          16
#define IRQ_RTCALARM_VECTOR     17
#define IRQ_RTCPERIOD_VECTOR    18
#define IRQ_TIMER1_VECTOR       19
#define IRQ_LCDC_VECTOR         20
#define IRQ_DMA_VECTOR          21
#define IRQ_RESERVE22_VECTOR    22
#define IRQ_RESERVE23_VECTOR    23
#define IRQ_APBBRIDGE_VECTOR    24
#define IRQ_ENETMAC_VECTOR      25
#define IRQ_RESERVE26_VECTOR    26
#define IRQ_EXTINT27_VECTOR     27
#define IRQ_EXTINT28_VECTOR     28
#define IRQ_RESERVE29_VECTOR    29
#define IRQ_RESERVE30_VECTOR    30
#define IRQ_RESERVE31_VECTOR    31

#ifndef __ASSEMBLER__

/*****************************************************************************
 * Device Specific Peripheral Registers structures
 ****************************************************************************/

#define __I                     volatile const  /* 'read only' permissions      */
#define __O                     volatile        /* 'write only' permissions     */
#define __IO                    volatile        /* 'read / write' permissions   */

/*****************************************************************************
 * AHBC - AG101P
 ****************************************************************************/
typedef struct {
	__IO unsigned int SLAVEBS[32];          /* 0x00 ~ 0x7C AHB Slave #n Base/Size Register */
	__IO unsigned int PLEVEL;               /* 0x80 Priority Control Register */
	__IO unsigned int IDLECNT;              /* 0x84 Idle Count Register */
	__IO unsigned int CTRL;                 /* 0x88 Control Register */
	__I  unsigned int REVISION;             /* 0x8C Revision Register */
} AG101P_AHBC_RegDef;

/*****************************************************************************
 * SMC - AG101P
 ****************************************************************************/
typedef struct {
	__IO unsigned int  CFG;                 /* Configuration Register of Memory Bank */
	__IO unsigned int  TIMING;              /* Timing Parameter Register of Memory Bank */
} SMC_BANK_REG;

typedef struct {
	SMC_BANK_REG      BANK[4];              /* 0x00 ~ 0x3c Bank #n Registers */
	__IO unsigned int SSR;                  /* 0x40 Shadow Status Register */
} AG101P_SMC_RegDef;

/*****************************************************************************
 * UARTx - AG101P
 ****************************************************************************/
typedef struct {
	union {
		__I  unsigned int RBR;          /* 0x00 Receiver Buffer Register */
		__O  unsigned int THR;          /* 0x00 Transmitter Holding Register */
		__IO unsigned int DLL;          /* 0x00 DLAB = 1, Baud Rate Divisor Latch Least Significant Byte */
	};
	union {
		__IO unsigned int IER;          /* 0x04 Interrupt Enable Register */
		__IO unsigned int DLM;          /* 0x04 DLAB = 1, Baud Rate Divisor Latch Most Significant Byte */
	};
	union {
		__I  unsigned int IIR;          /* 0x08 Interrupt Identification Register */
		__O  unsigned int FCR;          /* 0x08 FIFO Control Register */
		__IO unsigned int PSR;          /* 0x08 DLAB = 1, Prescaler Register */
	};
		__IO unsigned int LCR;          /* 0x0C Line Control Register */
		__IO unsigned int MCR;          /* 0x10 Modem Control Register */
	union {
		__I  unsigned int LSR;          /* 0x14 Line Status Register */
		__O  unsigned int TST;          /* 0x14 Testing Register */
	};
	__I  unsigned int MSR;                  /* 0x18 Modem Status Register */
	__IO unsigned int SPR;                  /* 0x1C Scratch Pad Register */
	__IO unsigned int MDR;                  /* 0x20 Mode Definition Register */
	__IO unsigned int ACR;                  /* 0x24 Auxiliary Control Register */
	__IO unsigned int TXLENL;               /* 0x28 Transmitter Frame Length Low */
	__IO unsigned int TXLENH;               /* 0x2C Transmitter Frame Length High */
	__IO unsigned int MRXLENL;              /* 0x30 Maximum Receiver Frame Length Low */
	__IO unsigned int MRXLENH;              /* 0x34 Maximum Receiver Frame Length High */
	__IO unsigned int PLR;                  /* 0x38 FIR Preamble Length Register */
        union {
		__O  unsigned int FMIIR_PIO;    /* 0x3C FIR Mode Interrupt Identification Register In PIO mode */
		__O  unsigned int FMIIR_DMA;    /* 0x3C FIR Mode Interrupt Identification Register In DMA mode */
	};
        union {
		__IO unsigned int FMIIER_PIO;   /* 0x40 FIR Mode Interrupt Enable Register for PIO Mode */
		__IO unsigned int FMIIER_DMA;   /* 0x40 FIR Mode Interrupt Enable Register in DMA mode */
        };
	__O  unsigned int STFF_STS;             /* 0x44 Status FIFO Line Status Register */
	__O  unsigned int STFF_RXLENL;          /* 0x48 Status FIFO Received Frame Length Register Low */
	__O  unsigned int STFF_RXLENH;          /* 0x4C Status FIFO Received Frame Length Register Low */
	__O  unsigned int FMLSR;                /* 0x50 FIR Mode Link Status Register */
	__IO unsigned int FMLSIER;              /* 0x54 FIR Mode Link Status Interrupt Enable Register */
	__O  unsigned int RSR;                  /* 0x58 Resume Register */
	__O  unsigned int RXFF_CNTR;            /* 0x5C Rx FIFO Count Register */
	__IO unsigned int LSTFMLENL;            /* 0x60 Last Frame Length Register Low */
	__IO unsigned int LSTFMLENH;            /* 0x64 Last Frame Length Register High */
	__O  unsigned int FEATURE;              /* 0x68 Feature Register*/
	__O  unsigned int REVD1;                /* 0x6C Revision Register Digit 1 */
	__O  unsigned int REVD2;                /* 0x70 Revision Register Digit 2 */
	__O  unsigned int REVD3;                /* 0x74 Revision Register Digit 3 */
} AG101P_UART_RegDef;

/*****************************************************************************
 * PWM - AG101P
 ****************************************************************************/
typedef struct {
	__IO uint32_t  CTRL;                    /* Control Registers for the #n channel output */
	__IO uint32_t  DUTY;                    /* Duty cycle Registers for the #n channel output */
	__IO uint32_t  PERVAL;                  /* Period Control for the #n channel output */
	__IO uint32_t  RESERVED[1];             /* Reserved */
} PWM_CHANNEL_REG;

typedef struct {
	PWM_CHANNEL_REG   CHANNEL[4];           /* 0x00 ~ 0x3c Channel #n Registers */
} AG101P_PWM_RegDef;

/*****************************************************************************
 * RTC - AG101P
 ****************************************************************************/
typedef struct _AG101P_RTC_RegDef{
	__I  unsigned int RTCSEC;               /* 0x00 Second Register */
	__I  unsigned int RTCMIN;               /* 0x04 Minute Register */
	__I  unsigned int RTCHR;                /* 0x08 Hour Register */
	__I  unsigned int RTCDAY;               /* 0x0C Day Count Register */
	__IO unsigned int ALMSEC;               /* 0x10 Second Alarm Register */
	__IO unsigned int ALMMIN;               /* 0x14 Minute Alarm Register */
	__IO unsigned int ALMHR;                /* 0x18 Hour Alarm Register */
	__IO unsigned int RESERVED[1];          /* 0x1C Reserved */
	__IO unsigned int CTRL;                 /* 0x20 Control Register */
	__IO unsigned int WRITESEC;             /* 0x24 Second counter write port */
	__IO unsigned int WRITEMIN;             /* 0x28 Minute counter write port */
	__IO unsigned int WRITEHR;              /* 0x2C Hour counter write port */
	__IO unsigned int WRITEDAY;             /* 0x30 Day counter write port */
	__IO unsigned int INTRSTATUS;           /* 0x34 Interrupt Register */
	__IO unsigned int DIV;                  /* 0x38 Frequency Divider */
	__IO unsigned int REVISION;             /* 0x3C Revision Register */
} AG101P_RTC_RegDef;

/*****************************************************************************
 * GPIO - AG101P
 ****************************************************************************/
typedef struct {
	__IO unsigned int DATAOUT;              /* 0x00 Data Output Register */
	__I  unsigned int DATAIN;               /* 0x04 Data Input Register */
	__IO unsigned int CHANNELDIR;           /* 0x08 Direction Register */
	__IO unsigned int PINBYPASS;            /* 0x0C Bypass Register */
	__O  unsigned int DATASET;              /* 0x10 Data bit set Register */
	__O  unsigned int DATACLEAR;            /* 0x14 Data bit clear Register */
	__IO unsigned int PULLEN;               /* 0x18 Pull-up Register */
	__IO unsigned int PULLTYPE;             /* 0x1C Pull-High/Pull-Low Register */
	__IO unsigned int INTREN;               /* 0x20 Interrupt Enable Register */
	__I  unsigned int INTRSTATUS;           /* 0x24 Interrupt Raw Status Register */
	__I  unsigned int INTRMASKSTATE;        /* 0x28 Interrupt Masked status Register */
	__IO unsigned int INTRMASK;             /* 0x2C Interrupt Mask Register */
	__O  unsigned int INTRCLEAR;            /* 0x30 Interrupt Clear Register */
	__IO unsigned int INTRTRIG;             /* 0x34 Interrupt Trigger Method Register */
	__IO unsigned int INTRBOTH;             /* 0x38 Edge-trigger interrupt by single or both edges */
	__IO unsigned int INTRISENEG;           /* 0x3C Interrupt triggered at the rising or falling edge */
	__IO unsigned int BOUNCEEN;             /* 0x40 Pre-scale clock enable */
	__IO unsigned int BOUNCEPRESCALE;       /* 0x44 Pre-scale, used to adjust different PCLK frequencies */
	__IO unsigned int RESERVED[13];
	__I  unsigned int REVISION;             /* 0x7C Revision number */
} AG101P_GPIO_RegDef;

/*****************************************************************************
 * APBBRIDGE - AG101P
 ****************************************************************************/
typedef struct {
	__IO unsigned int SRCADDR;              /* Channel Source Register */
	__IO unsigned int DSTADDR;              /* Channel Destination Register */
	__IO unsigned int CYCLES;               /* Channel Cycles Register */
	__IO unsigned int CMD;                  /* Channel Command Register */
} APBBRIDGE_CHANNEL_REG;

typedef struct {
	__IO unsigned int SLAVE[32];            /* 0x00 ~ 0x80 Slave #n Base/Size Register */
	APBBRIDGE_CHANNEL_REG CHANNEL[4];
	__I  unsigned int REVISION;             /* 0xC0 Revision Register */
	__I  unsigned int RESERVED[3];          /* 0xC4~0xCC Reserve Register */
	__IO unsigned int MISC;                 /* 0xD0 Revision Register */
} AG101P_APBBRIDGE_RegDef;

/*****************************************************************************
 * TMR - AG101P
 ****************************************************************************/
typedef struct {
	__IO unsigned int COUNTER;              /* TMR Channel Counter Register */
	__IO unsigned int RELOAD;               /* TMR Channel Reload Register */
	__IO unsigned int MATCH1;               /* TMR Channel Match1 Register */
	__IO unsigned int MATCH2;               /* TMR Channel Match2 Register */
} TMR_CHANNEL_REG;

typedef struct {
	__IO unsigned int SCALER;               /* TMR Timer Prescaler Register */
} TMR_CHANNEL_PRESCALER;

typedef struct {
	TMR_CHANNEL_REG   CHANNEL[3];           /* 0x00 ~ 0x2C Channel #n Registers */
	__IO unsigned int CTRL;                 /* 0x30 TMR Channel Control Register */
	__IO unsigned int INTST;                /* 0x34 Interrupt Status Register */
	__IO unsigned int INTMSK;               /* 0x38 Interrupt Enable Register */
	__I  unsigned int IDREV;                /* 0x3C ID and Revision Register */
	TMR_CHANNEL_PRESCALER PRESCALER[3];     /* 0x40 ~ 0x48 prescaler */
} AG101P_TMR_RegDef;

/*****************************************************************************
 * PMU - AG101P
 ****************************************************************************/
typedef struct {
	__I  unsigned int IDREV;                /* 0x00 ID and Revision Register */
	     unsigned int RESERVED0[19];        /* 0x04 ~ 0x4C Reserved */
	__IO unsigned int SCRATCH[16];          /* 0x50 ~ 0x8C Scratch Pad Register */
	__I  unsigned int CKRATIO;              /* 0x90 Clock Ratio Register */
	     unsigned int RESERVED1[27];        /* 0x94 ~ 0xFC Reserved */
	__I  unsigned int DEVCFG;               /* 0x100 Clock Ratio Register */
	__I  unsigned int BOARDID;              /* 0x104 Board ID Register Register */
} AG101P_PMU_RegDef;

/*****************************************************************************
 * EXT INTC - AG101P
 ****************************************************************************/
typedef struct {
	__I  unsigned int IRQ_SRC;              /* 0x00 HW1 (IRQ) Source Register */
	__IO unsigned int IRQ_EN;               /* 0x04 HW1 (IRQ) Enable Register */
	__IO unsigned int IRQ_INT_CLR;          /* 0x08 HW1 (IRQ) Interrupt Clear Register */
	__IO unsigned int IRQ_TRG_MODE;         /* 0x0C HW1 (IRQ) Trigger-mode Register */
	__IO unsigned int IRQ_TRG_LVL;          /* 0x10 HW1 (IRQ) Trigger-level Register */
	__I  unsigned int IRQ_STATUS;           /* 0x14 HW1 (IRQ) Status Register */
	__IO unsigned int RESERVED0[2];         /* 0x18 ~ 0x1C Reserved */
	__I  unsigned int FIQ_SRC;              /* 0x20 HW0 (FIQ) Source Register */
	__IO unsigned int FIQ_EN;               /* 0x24 HW0 (FIQ) Enable Register */
	__IO unsigned int FIQ_INT_CLR;          /* 0x28 HW0 (FIQ) Interrupt Clear Register */
	__IO unsigned int FIQ_TRG_MODE;         /* 0x2C HW0 (FIQ) Irigger-mode Register */
	__IO unsigned int FIQ_TRG_LVL;          /* 0x30 HW0 (FIQ) Trigger-level Register */
	__I  unsigned int FIQ_STATUS;           /* 0x34 HW0 (FIQ) Status Register */
	__IO unsigned int RESERVED1[2];         /* 0x38 ~ 0x3C Reserved */
	__IO unsigned int RESERVED2[4];         /* 0x40 ~ 0x4C Reserved */
	__I  unsigned int REVISION;             /* 0x50 Revision Register */
	__IO unsigned int RESERVED3[3];         /* 0x54 ~ 0x5C Reserved */
	__IO unsigned int EVIC_INT_PRI0_7;      /* 0x60 EVIC interrupt priority level configuration for bit0~bit7 */
	__IO unsigned int EVIC_INT_PRI8_15;     /* 0x64 EVIC interrupt priority level configuration for bit8~bit15 */
	__IO unsigned int EVIC_INT_PRI16_23;    /* 0x68 EVIC interrupt priority level configuration for bit16~bit23 */
	__IO unsigned int EVIC_INT_PRI24_31;    /* 0x6C EVIC interrupt priority level configuration for bit24~bit31 */
} AG101P_INTC_RegDef;

/*****************************************************************************
 * Memory Map
 ****************************************************************************/
#if (defined(CFG_XIP) && (defined(CFG_CACHE_ENABLE)))
/*
 * The NTC3 is set to noncache region and NTM3 is mapped to partition 2 (I/O region).
 * Map the I/O address to NTC3 to be uncachable.
 */
#define UNCACHE_MAP(addr)       ((addr) | 0xC0000000)
#else
#define UNCACHE_MAP(addr)       (addr)
#endif

#define _IO_(addr)              UNCACHE_MAP(addr)

#ifdef CFG_16MB
#define AHBC_BASE               _IO_(0x00E00000)
#define SMC_BASE                _IO_(0x00E01000)
#define MEMC_BASE               _IO_(0x00E02000)
#define DMAC_BASE               _IO_(0x00E03000)
#define APBBRG_BASE             _IO_(0x00E04000)
#define MAC_BASE                _IO_(0x00E05000)
#define LCDC_BASE               _IO_(0x00E10000)
#define PMU_BASE                _IO_(0x00F01000)
#define UART1_BASE              _IO_(0x00F03000)
#define TMRC_BASE               _IO_(0x00F04000)
#define WDT_BASE                _IO_(0x00F05000)
#define RTC_BASE                _IO_(0x00F06000)
#define GPIO_BASE               _IO_(0x00F07000)
#define INTC_BASE               _IO_(0x00F08000)
#define I2C_BASE                _IO_(0x00F0A000)
#define SSP1_BASE               _IO_(0x00F0B000)
#define SDC_BASE                _IO_(0x00F0E000)
#define PWM_BASE                _IO_(0x00F11000)
#define SSP2_BASE               _IO_(0x00F14000)
#define SSP3_BASE               _IO_(0x00F15000)
#define UART2_BASE              _IO_(0x00F16000)
#else
#define AHBC_BASE               _IO_(0x90100000)
#define SMC_BASE                _IO_(0x90200000)
#define MEMC_BASE               _IO_(0x90300000)
#define DMAC_BASE               _IO_(0x90400000)
#define APBBRG_BASE             _IO_(0x90500000)
#define LCDC_BASE               _IO_(0x90600000)
#define MAC_BASE                _IO_(0x90900000)
#define PMU_BASE                _IO_(0x98100000)
#define UART1_BASE              _IO_(0x98300000)
#define TMRC_BASE               _IO_(0x98400000)
#define WDT_BASE                _IO_(0x98500000)
#define RTC_BASE                _IO_(0x98600000)
#define GPIO_BASE               _IO_(0x98700000)
#define INTC_BASE               _IO_(0x98800000)
#define I2C_BASE                _IO_(0x98A00000)
#define SSP1_BASE               _IO_(0x98B00000)
#define SDC_BASE                _IO_(0x98E00000)
#define PWM_BASE                _IO_(0x99100000)
#define SSP2_BASE               _IO_(0x99400000)
#define SSP3_BASE               _IO_(0x99500000)
#define UART2_BASE              _IO_(0x99600000)
#endif

/*****************************************************************************
 * Peripheral declaration
 ****************************************************************************/
#define AG101P_AHBC             ((AG101P_AHBC_RegDef *) AHBC_BASE)
#define AG101P_SMC              ((AG101P_SMC_RegDef *)  SMC_BASE)
#define AG101P_TMR              ((AG101P_TMR_RegDef *)  TMRC_BASE)
#define AG101P_INTC             ((AG101P_INTC_RegDef *) INTC_BASE)
#define AG101P_UART1            ((AG101P_UART_RegDef *) UART1_BASE)
#define AG101P_UART2            ((AG101P_UART_RegDef *) UART2_BASE)
#define AG101P_GPIO             ((AG101P_GPIO_RegDef *) GPIO_BASE)
#define AG101P_DMA              ((AG101P_DMA_RegDef *)  DMAC_BASE)
#define AG101P_APBBRG           ((AG101P_APBBRIDGE_RegDef *) APBBRG_BASE)
#define AG101P_PMU              ((AG101P_PMU_RegDef *)  PMU_BASE)
#define AG101P_RTC              ((AG101P_RTC_RegDef *)  RTC_BASE)
#define AG101P_PWM              ((AG101P_PWM_RegDef *)  PWM_BASE)

#define PLAT_UART               AG101P_UART2

#endif	/* __ASSEMBLER__ */

#endif	/* __AG101P_H__ */
