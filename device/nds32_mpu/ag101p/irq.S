! -----------------------------------------------------------------------------
! irq.S
!
! Copyright (c) 2005-2018, Andes Techonology Corporation
! All rights reserved.
! -----------------------------------------------------------------------------

#include "ag101p.h"
#include "ISR_Support.h"

!********************************************************************************************************
!	Setup HW Interrupt vector handler
!********************************************************************************************************

	.macro  SET_HWISR num, handler
	.global interrupt_HW\num
	.set    interrupt_HW\num, \handler
	.endm

	! Setup the AG101P HW interrupt vector handler
	SET_HWISR IRQ_AHBC_NOSLAVE_VECTOR,	interrupt_comm_vsr
	SET_HWISR IRQ_RESERVE1_VECTOR,          interrupt_comm_vsr
	SET_HWISR IRQ_SSP1_VECTOR,		interrupt_comm_vsr
	SET_HWISR IRQ_I2C_VECTOR,		interrupt_comm_vsr
	SET_HWISR IRQ_CPU_LM_DMA_VECTOR,	interrupt_comm_vsr
	SET_HWISR IRQ_SDC_VECTOR,		interrupt_comm_vsr
	SET_HWISR IRQ_SSP2_VECTOR,		interrupt_comm_vsr
	SET_HWISR IRQ_UART2_VECTOR,		interrupt_comm_vsr
	SET_HWISR IRQ_PMU_VECTOR,		interrupt_comm_vsr
	SET_HWISR IRQ_CPU_PFM_VECTOR,		interrupt_comm_vsr
	SET_HWISR IRQ_UART1_VECTOR,		interrupt_comm_vsr
	SET_HWISR IRQ_RESERVE12_VECTOR,		interrupt_comm_vsr
	SET_HWISR IRQ_GPIO_VECTOR,		interrupt_hipri_vsr
	SET_HWISR IRQ_TIMER2_VECTOR,		interrupt_comm_vsr
	SET_HWISR IRQ_TIMER3_VECTOR,		interrupt_comm_vsr
	SET_HWISR IRQ_WDT_VECTOR,		interrupt_comm_vsr
	SET_HWISR IRQ_RTCALARM_VECTOR,		interrupt_hipri_vsr
	SET_HWISR IRQ_RTCSECOND_VECTOR,		interrupt_hipri_vsr
	SET_HWISR IRQ_TIMER1_VECTOR,		interrupt_comm_vsr
	SET_HWISR IRQ_LCDC_VECTOR,		interrupt_comm_vsr
	SET_HWISR IRQ_DMA_VECTOR,		interrupt_comm_vsr
	SET_HWISR IRQ_RESERVE22_VECTOR,		interrupt_comm_vsr
	SET_HWISR IRQ_RESERVE23_VECTOR,		interrupt_comm_vsr
	SET_HWISR IRQ_APBBRIDGE_VECTOR,		interrupt_comm_vsr
	SET_HWISR IRQ_ENETMAC_VECTOR,		interrupt_comm_vsr
	SET_HWISR IRQ_RESERVE26_VECTOR,		interrupt_comm_vsr
	SET_HWISR IRQ_EXTINT27_VECTOR,          interrupt_comm_vsr
	SET_HWISR IRQ_EXTINT28_VECTOR,          interrupt_comm_vsr
	SET_HWISR IRQ_RESERVE29_VECTOR,		interrupt_comm_vsr
	SET_HWISR IRQ_RESERVE30_VECTOR,		interrupt_comm_vsr
	SET_HWISR IRQ_RESERVE31_VECTOR,		interrupt_comm_vsr

!********************************************************************************************************
!       Highest priority HW Interrupt vector service routine (Non-Nested interrupt)
!********************************************************************************************************

interrupt_hipri_vsr:
	! We enter here with the orginal r0~r5 is saved and
	! r0 is HW interrup vector number.

	pushm   $r0, $r5
	pushm   $r15,$r30

        portSAVE_FPU_CALLER

#if ( configSUPPORT_IFC == 1 )
	mfusr   $r1, $IFC_LP
	pushm   $r0, $r1                /* $0 is dummy for 8-byte alignment stack pointer */
#endif

	move    $r0, $p1                /* IRQ number */

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
	WEAK_DEFAULT ahbc_irq_handler,   	default_irq_handler
	WEAK_DEFAULT rsv1_irq_handler,		default_irq_handler
	WEAK_DEFAULT ssp1_irq_handler,		default_irq_handler
	WEAK_DEFAULT i2c_irq_handler,   	default_irq_handler
	WEAK_DEFAULT cpu_lm_dma_irq_handler,	default_irq_handler
	WEAK_DEFAULT sdc_irq_handler,   	default_irq_handler
	WEAK_DEFAULT ssp2_irq_handler,   	default_irq_handler
	WEAK_DEFAULT uart2_irq_handler,   	default_irq_handler
	WEAK_DEFAULT pmu_irq_handler,   	default_irq_handler
	WEAK_DEFAULT swi_irq_handler,   	default_irq_handler
	WEAK_DEFAULT cpu_pfm_irq_handler,   	default_irq_handler
	WEAK_DEFAULT uart1_irq_handler,   	default_irq_handler
	WEAK_DEFAULT rsv12_irq_handler,   	default_irq_handler
	WEAK_DEFAULT gpio_irq_handler,   	default_irq_handler
	WEAK_DEFAULT tmr2_irq_handler,   	default_irq_handler
	WEAK_DEFAULT tmr3_irq_handler,   	default_irq_handler
	WEAK_DEFAULT wdt_irq_handler,   	default_irq_handler
	WEAK_DEFAULT rtc_alarm_irq_handler,   	default_irq_handler
	WEAK_DEFAULT rtc_second_irq_handler,   	default_irq_handler
	WEAK_DEFAULT tmr1_irq_handler,          FreeRTOS_Tick_Handler /* FreeRTOS_Tick_Handler() is installed as PIT interrupt handler */
	WEAK_DEFAULT lcdc_irq_handler,   	default_irq_handler
	WEAK_DEFAULT dma_irq_handler,   	default_irq_handler
	WEAK_DEFAULT rsv22_irq_handler,   	default_irq_handler
	WEAK_DEFAULT rsv23_irq_handler,   	default_irq_handler
	WEAK_DEFAULT apb_bridge_irq_handler,   	default_irq_handler
	WEAK_DEFAULT enetmac_irq_handler,   	default_irq_handler
	WEAK_DEFAULT rsv26_irq_handler,   	default_irq_handler
	WEAK_DEFAULT extint27_irq_handler,   	default_irq_handler
	WEAK_DEFAULT extint28_irq_handler,   	default_irq_handler
	WEAK_DEFAULT rsv29_irq_handler,   	default_irq_handler
	WEAK_DEFAULT rsv30_pfm_irq_handler,   	default_irq_handler
	WEAK_DEFAULT rsv31_irq_handler,   	default_irq_handler

!********************************************************************************************************
!       Interrupt handler Table
!********************************************************************************************************

	.section .data
	.align  2

	! These tables contain the isr pointers used to deliver interrupts
	.global ISR_TABLE
ISR_TABLE:
	.long   ahbc_irq_handler          	!  0: AHBC
	.long   rsv1_irq_handler           	!  1: Reserved1
	.long   ssp1_irq_handler		!  2: SSP1
	.long   i2c_irq_handler			!  3: I2C
	.long   cpu_lm_dma_irq_handler		!  4: CPU LM DMA
	.long   sdc_irq_handler                 !  5: SDC
	.long   ssp2_irq_handler                !  6: SSP2
	.long   uart2_irq_handler               !  7: UART2
	.long   pmu_irq_handler			!  8: PMU
	.long   swi_irq_handler                 !  9: SWI
	.long   cpu_pfm_irq_handler		! 10: CPU PFM
	.long   uart1_irq_handler		! 11: UART1
	.long   rsv12_irq_handler		! 12: Reserved12
	.long   gpio_irq_handler                ! 13: GPIO
	.long   tmr2_irq_handler		! 14: TMR2
	.long   tmr3_irq_handler		! 15: TMR3
	.long   wdt_irq_handler			! 16: WDT
	.long   rtc_alarm_irq_handler		! 17: RTC ALARM
	.long   rtc_second_irq_handler		! 18: RTC SECOND
	.long   tmr1_irq_handler                ! 19: TMR1
	.long   lcdc_irq_handler		! 20: LCDC
	.long   dma_irq_handler           	! 21: DMA
	.long   rsv22_irq_handler		! 22: Reserved22
	.long   rsv23_irq_handler		! 23: Reserved23
	.long   apb_bridge_irq_handler		! 24: APB BRIDGE
	.long   enetmac_irq_handler             ! 25: ENETMAC
	.long   rsv26_irq_handler             	! 26: Reserved26
	.long   extint27_irq_handler            ! 27: EXT INT27
	.long   extint28_irq_handler		! 28: EXT INT28
	.long   rsv29_irq_handler               ! 29: Reserved29
	.long   rsv30_pfm_irq_handler		! 30: Reserved30
	.long   rsv31_irq_handler		! 31: Reserved31

	! Interrupt stack
	.section .bss

	.skip 1024
	.align 3
xISRStack:
	.end
