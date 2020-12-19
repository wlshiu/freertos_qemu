! -----------------------------------------------------------------------------
! irq.S
!
! Copyright (c) 2005-2018, Andes Techonology Corporation
! All rights reserved.
! -----------------------------------------------------------------------------

#include "ae3xx.h"
#include "ISR_Support.h"

!********************************************************************************************************
!	Setup HW Interrupt vector handler
!********************************************************************************************************

	.macro  SET_HWISR num, handler
	.global interrupt_HW\num
	.set    interrupt_HW\num, \handler
	.endm

	! Setup the AE3XX HW interrupt vector handler

	SET_HWISR IRQ_RTCPERIOD_VECTOR, interrupt_hipri_vsr
	SET_HWISR IRQ_RTCALARM_VECTOR,  interrupt_hipri_vsr
	SET_HWISR IRQ_PIT_VECTOR,       interrupt_comm_vsr
	SET_HWISR IRQ_SPI1_VECTOR,      interrupt_comm_vsr
	SET_HWISR IRQ_SPI2_VECTOR,      interrupt_comm_vsr
	SET_HWISR IRQ_I2C_VECTOR,       interrupt_comm_vsr
	SET_HWISR IRQ_GPIO_VECTOR,      interrupt_hipri_vsr
	SET_HWISR IRQ_UART1_VECTOR,     interrupt_comm_vsr
	SET_HWISR IRQ_UART2_VECTOR,     interrupt_comm_vsr
	SET_HWISR IRQ_DMA_VECTOR,       interrupt_comm_vsr
	SET_HWISR IRQ_RESERVED10_VECTOR,interrupt_comm_vsr
	!SET_HWISR IRQ_SWI_VECTOR,	interrupt_comm_vsr
	SET_HWISR IRQ_LDMA_VECTOR,      interrupt_comm_vsr
	SET_HWISR IRQ_PFMC_VECTOR,      interrupt_comm_vsr
	SET_HWISR IRQ_RESERVED14_VECTOR,interrupt_comm_vsr
	SET_HWISR IRQ_RESERVED15_VECTOR,interrupt_comm_vsr
	SET_HWISR IRQ_SSP1_VECTOR,      interrupt_comm_vsr
	SET_HWISR IRQ_SDC_VECTOR,       interrupt_comm_vsr
	SET_HWISR IRQ_ENETMAC_VECTOR,   interrupt_comm_vsr
	SET_HWISR IRQ_LCDC_VECTOR,      interrupt_comm_vsr
	SET_HWISR IRQ_USB_VECTOR,       interrupt_comm_vsr

!********************************************************************************************************
!       Highest priority HW Interrupt vector service routine (Non-Nested interrupt)
!********************************************************************************************************

interrupt_hipri_vsr:
	pushm   $r0, $r5
	pushm   $r15,$r30

	portSAVE_FPU_CALLER

#if ( configSUPPORT_IFC == 1 )
	mfusr   $r1, $IFC_LP
        pushm   $r0, $r1                /* $0 is dummy for 8-byte alignment stack pointer */
#endif

	! The entire CPU state is now stashed on the stack,
	! and the stack is also 8-byte alignment.
	! We can call C program based interrupt handler now.
	la      $r1, ISR_TABLE
	lw      $r1, [$r1+($r0<<2)]             ! ISR function pointer
	jral    $r1

#if ( configSUPPORT_IFC == 1 )
	popm    $r0, $r1
	mtusr   $r1, $IFC_LP
#endif

	portRESTORE_FPU_CALLER

	popm    $r15,$r30
	popm    $r0, $r5

	iret


!********************************************************************************************************
!       HW Interrupt vector service routine (Nested interrupt)
!********************************************************************************************************

interrupt_comm_vsr:
	pushm   $r0, $r5
	pushm   $r15,$r30

	portSAVE_FPU_CALLER

	/* Save system registers */
	mfsr    $r1, $IPC
	mfsr    $r2, $IPSW

#if ( configSUPPORT_IFC == 1 ) && ( configSUPPORT_ZOL == 1)
	pushm	$r1, $r2

	mfusr   $r1, $LB
	mfusr   $r2, $LE
	mfusr   $r3, $LC
	mfusr   $r4, $IFC_LP
	pushm   $r1, $r4
#else
#if ( configSUPPORT_IFC == 1 )
	mfusr   $r3, $IFC_LP
	pushm   $r0, $r3                 /* $0 is dummy for 8-byte alignment stack pointer */
#elif ( configSUPPORT_ZOL == 1 )
	mfusr   $r3, $LB
        mfusr   $r4, $LE
        mfusr   $r5, $LC
	pushm   $r0, $r5                 /* $0 is dummy for 8-byte alignment stack pointer */
#else
	pushm   $r1, $r2
#endif
#endif

	move    $r0, $p1                 /* IRQ number */

	/* Descend interrupt level for nested */
	mfsr    $r1, $PSW
#if ( configSUPPORT_ZOL == 1 )
	/* Also enable ZOL (PSW.AEN) */
	xori    $r1, $r1, #((1 << 13) | (1 << 1))
#else
	addi    $r1, $r1, #-2
#endif
	mtsr    $r1, $PSW

	/* Check reentrant interrupt */
	srli45  $r2, #16
	fexti33 $r2, #0x2                       ! IPSW.CPL
	bnec    $r2, #0x7, 2f                   ! IPSW.CPL != 7, come form ISR, reentrant

	move    $fp, $sp                        ! save old stack pointer
	la      $sp, xISRStack                  ! switch to interrupt stack
2:
	setgie.e                                ! allow nested now

	! The entire CPU state is now stashed on the stack,
	! and the stack is also 8-byte alignment.
	! We can call C program based interrupt handler now.
	la      $r1, ISR_TABLE
	lw      $r1, [$r1+($r0<<2)]             ! ISR function pointer
	jral    $r1                             ! Call ISR

	la      $r1, xISRStack                  ! Check for nested interruption return
	bne     $r1, $sp, 3f                    ! $sp != inetrrupt satck ?

	move    $sp, $fp                        ! Move back to the thread stack
3:
	setgie.d
	dsb

	/* Restore system registers */
#if ( configSUPPORT_IFC == 1 ) && ( configSUPPORT_ZOL == 1)
	popm	$r1, $r4
	mtusr   $r1, $LB
	mtusr   $r2, $LE
	mtusr   $r3, $LC
	mtusr   $r4, $IFC_LP

	popm    $r1, $r2
#else
#if ( configSUPPORT_IFC == 1 )
	popm    $r0, $r3
	mtusr   $r3, $IFC_LP
#elif ( configSUPPORT_ZOL == 1 )
	popm    $r0, $r5
	mtusr   $r3, $LB
    mtusr   $r4, $LE
    mtusr   $r5, $LC
#else
	popm    $r1, $r2
#endif
#endif

	mtsr    $r1, $IPC
	mtsr    $r2, $IPSW

	portRESTORE_FPU_CALLER

	popm    $r15,$r30
	popm    $r0, $r5

	iret


!********************************************************************************************************
!       Interrupt default handler
!********************************************************************************************************

	! Macro to define default handlers
	.macro WEAK_DEFAULT weak_sym, default_handler
	.weak \weak_sym
	.set \weak_sym ,\default_handler
	.endm

	! Default irq handler
default_irq_handler:
	b       default_irq_handler

	! Define default handler to be weak symbol.
	! They can be overwritten by other handlers
	WEAK_DEFAULT rtc_period_irq_handler,   default_irq_handler
	WEAK_DEFAULT rtc_alarm_irq_handler,    default_irq_handler
	WEAK_DEFAULT pit_irq_handler,          FreeRTOS_Tick_Handler /* FreeRTOS_Tick_Handler() is installed as PIT interrupt handler */
	WEAK_DEFAULT spi1_irq_handler,         default_irq_handler
	WEAK_DEFAULT spi2_irq_handler,         default_irq_handler
	WEAK_DEFAULT i2c_irq_handler,          default_irq_handler
	WEAK_DEFAULT gpio_irq_handler,         default_irq_handler
	WEAK_DEFAULT uart1_irq_handler,        default_irq_handler
	WEAK_DEFAULT uart2_irq_handler,        default_irq_handler
	WEAK_DEFAULT dma_irq_handler           default_irq_handler
	WEAK_DEFAULT rsv10_irq_handler         default_irq_handler
	WEAK_DEFAULT swi_irq_handler           default_irq_handler
	WEAK_DEFAULT ldma_irq_handler          default_irq_handler
	WEAK_DEFAULT pfmc_irq_handler          default_irq_handler
	WEAK_DEFAULT rsv14_irq_handler         default_irq_handler
	WEAK_DEFAULT rsv15_irq_handler         default_irq_handler
	WEAK_DEFAULT ssp1_irq_handler          default_irq_handler
	WEAK_DEFAULT sdc_irq_handler           default_irq_handler
	WEAK_DEFAULT enetmac_irq_handler       default_irq_handler
	WEAK_DEFAULT lcdc_irq_handler          default_irq_handler
	WEAK_DEFAULT usb_irq_handler           default_irq_handler

!********************************************************************************************************
!       Interrupt handler Table
!********************************************************************************************************

	.section .data
	.align  2

	! These tables contain the isr pointers used to deliver interrupts
	.global ISR_TABLE
ISR_TABLE:
	.long   rtc_period_irq_handler          !  0: RTC period
	.long   rtc_alarm_irq_handler           !  1: RTC alarm
	.long   pit_irq_handler                 !  2: PIT
	.long   spi1_irq_handler                !  3: SPI1
	.long   spi2_irq_handler                !  4: SPI2
	.long   i2c_irq_handler                 !  5: I2C
	.long   gpio_irq_handler                !  6: GPIO
	.long   uart1_irq_handler               !  7: UART1
	.long   uart2_irq_handler               !  8: UART2
	.long   dma_irq_handler                 !  9: DMA
	.long   rsv10_irq_handler               ! 10: Reserved10
	.long   swi_irq_handler                 ! 11: SWINT
	.long   ldma_irq_handler                ! 12: CPU_LDMA
	.long   pfmc_irq_handler                ! 13: PFMC
	.long   rsv14_irq_handler               ! 14: Reserved14
	.long   rsv15_irq_handler               ! 15: Reserved15
	.long   ssp1_irq_handler                ! 16: SSP1,AC97/IIS
	.long   sdc_irq_handler                 ! 17: SDC
	.long   enetmac_irq_handler             ! 18: ENETMAC
	.long   lcdc_irq_handler                ! 19: LCDC
	.long   usb_irq_handler                 ! 20: USB

	! Interrupt stack
	.section .bss

	.skip 1024
	.align 3
xISRStack:
	.end
