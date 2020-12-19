/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

	#include "ISR_Support.h"

	.global vRegTest1Implementation
	.global vRegTest2Implementation
	.extern ullRegTest1LoopCounter
	.extern ullRegTest2LoopCounter

	.text

	/* This function is explained in the comments at the top of main-full.c. */
.type vRegTest1Implementation, %function
vRegTest1Implementation:

	/* Fill each general purpose register with a known value. */
	move	$r0, #0xff
	move	$r1, #0x01
	move	$r2, #0x02
	move	$r3, #0x03
	move	$r4, #0x04
	move	$r5, #0x05
	move	$r6, #0x06
	move	$r7, #0x07
	move	$r8, #0x08
	move	$r9, #0x09
	move	$r10, #0x10
#ifdef __NDS32_REDUCE_REGS__
	move	$r15, #0x15
#else
	move	$r11, #0x11
	move	$r12, #0x12
	move	$r13, #0x13
	move	$r14, #0x14
	move	$r15, #0x15
	move	$r16, #0x16
	move	$r17, #0x17
	move	$r18, #0x18
	move	$r19, #0x19
	move	$r20, #0x20
	move	$r21, #0x21
	move	$r22, #0x22
	move	$r23, #0x23
	move	$r24, #0x24
	move	$r25, #0x25
	/* $26/$27 ($p0/$p1) is unused */
#endif
	move	$r28, #0x28
	/* $29 ($gp) is the Global pointer  - don't mess with $29. */
	move	$r30, #0x30

	/* Fill each FPU register with a known value. */
#if ( configSUPPORT_FPU == 1 )
	fmtdr	$r0, $fd0
	fmtdr	$r2, $fd1
	fmtdr	$r4, $fd2
	fmtdr	$r6, $fd3
	#if ( portFPU_REGS > 8 )
	fmtdr	$r8, $fd4
	fmtdr	$r10, $fd5
	fmtdr	$r12, $fd6
	fmtdr	$r14, $fd7
	#endif
	#if ( portFPU_REGS > 16 )
	fmtdr	$r0, $fd8
	fmtdr	$r2, $fd9
	fmtdr	$r4, $fd10
	fmtdr	$r6, $fd11
	fmtdr	$r8, $fd12
	fmtdr	$r10, $fd13
	fmtdr	$r12, $fd14
	fmtdr	$r14, $fd15
	#endif
	#if ( portFPU_REGS > 32 )
	fmtdr	$r0, $fd16
	fmtdr	$r2, $fd17
	fmtdr	$r4, $fd18
	fmtdr	$r6, $fd19
	fmtdr	$r8, $fd20
	fmtdr	$r10, $fd21
	fmtdr	$r12, $fd22
	fmtdr	$r14, $fd23
	fmtdr	$r0, $fd24
	fmtdr	$r2, $fd25
	fmtdr	$r4, $fd26
	fmtdr	$r6, $fd27
	fmtdr	$r8, $fd28
	fmtdr	$r10, $fd29
	fmtdr	$r12, $fd30
	fmtdr	$r14, $fd31
	#endif
#endif

	/* Loop, checking each itteration that each register still contains the
	expected value. */
reg1_loop:

	/* Test each general purpose register to check that it still contains the
	expected known value, jumping to reg1_error_loop if any register contains
	an unexpected value. */
	bnec	$r0, #0xFF, reg1_error_loop
	bnec	$r1, #0x01, reg1_error_loop
	bnec	$r2, #0x02, reg1_error_loop
	bnec	$r3, #0x03, reg1_error_loop
	bnec	$r4, #0x04, reg1_error_loop
	bnec	$r5, #0x05, reg1_error_loop
	bnec	$r6, #0x06, reg1_error_loop
	bnec	$r7, #0x07, reg1_error_loop
	bnec	$r8, #0x08, reg1_error_loop
	bnec	$r9, #0x09, reg1_error_loop
	bnec	$r10, #0x10, reg1_error_loop
#ifdef __NDS32_REDUCE_REGS__
	bnec	$r15, #0x15, reg1_error_loop
#else
	bnec	$r11, #0x11, reg1_error_loop
	bnec	$r12, #0x12, reg1_error_loop
	bnec	$r13, #0x13, reg1_error_loop
	bnec	$r14, #0x14, reg1_error_loop
	bnec	$r15, #0x15, reg1_error_loop
	bnec	$r16, #0x16, reg1_error_loop
	bnec	$r17, #0x17, reg1_error_loop
	bnec	$r18, #0x18, reg1_error_loop
	bnec	$r19, #0x19, reg1_error_loop
	bnec	$r20, #0x20, reg1_error_loop
	bnec	$r21, #0x21, reg1_error_loop
	bnec	$r22, #0x22, reg1_error_loop
	bnec	$r23, #0x23, reg1_error_loop
	bnec	$r24, #0x24, reg1_error_loop
	bnec	$r25, #0x25, reg1_error_loop
#endif
	bnec	$r28, #0x28, reg1_error_loop
	bnec	$r30, #0x30, reg1_error_loop

	/* Check every floating point register to ensure it contains the expected
	value.  First save the registers clobbered by the test. */
	pushm	$r0, $r1
#if ( configSUPPORT_FPU == 1 )
	fmfdr	$r0, $fd0
	bnec	$r0, #0xFF, reg1_error_loop
	bnec	$r1, #0x01, reg1_error_loop
	fmfdr	$r0, $fd1
	bnec	$r0, #0x02, reg1_error_loop
	bnec	$r1, #0x03, reg1_error_loop
	fmfdr	$r0, $fd2
	bnec	$r0, #0x04, reg1_error_loop
	bnec	$r1, #0x05, reg1_error_loop
	fmfdr	$r0, $fd3
	bnec	$r0, #0x06, reg1_error_loop
	bnec	$r1, #0x07, reg1_error_loop
	#if ( portFPU_REGS > 8 )
	fmfdr	$r0, $fd4
	bnec	$r0, #0x08, reg1_error_loop
	bnec	$r1, #0x09, reg1_error_loop
	fmfdr	$r0, $fd5
	bnec	$r0, #0x10, reg1_error_loop
	bnec	$r1, #0x11, reg1_error_loop
	fmfdr	$r0, $fd6
	bnec	$r0, #0x12, reg1_error_loop
	bnec	$r1, #0x13, reg1_error_loop
	fmfdr	$r0, $fd7
	bnec	$r0, #0x14, reg1_error_loop
	bnec	$r1, #0x15, reg1_error_loop
	#endif
	#if ( portFPU_REGS > 16 )
	fmfdr	$r0, $fd8
	bnec	$r0, #0xFF, reg1_error_loop
	bnec	$r1, #0x01, reg1_error_loop
	fmfdr	$r0, $fd9
	bnec	$r0, #0x02, reg1_error_loop
	bnec	$r1, #0x03, reg1_error_loop
	fmfdr	$r0, $fd10
	bnec	$r0, #0x04, reg1_error_loop
	bnec	$r1, #0x05, reg1_error_loop
	fmfdr	$r0, $fd11
	bnec	$r0, #0x06, reg1_error_loop
	bnec	$r1, #0x07, reg1_error_loop
	fmfdr	$r0, $fd12
	bnec	$r0, #0x08, reg1_error_loop
	bnec	$r1, #0x09, reg1_error_loop
	fmfdr	$r0, $fd13
	bnec	$r0, #0x10, reg1_error_loop
	bnec	$r1, #0x11, reg1_error_loop
	fmfdr	$r0, $fd14
	bnec	$r0, #0x12, reg1_error_loop
	bnec	$r1, #0x13, reg1_error_loop
	fmfdr	$r0, $fd15
	bnec	$r0, #0x14, reg1_error_loop
	bnec	$r1, #0x15, reg1_error_loop
	#endif
	#if ( portFPU_REGS > 32 )
	fmfdr	$r0, $fd16
	bnec	$r0, #0xFF, reg1_error_loop
	bnec	$r1, #0x01, reg1_error_loop
	fmfdr	$r0, $fd17
	bnec	$r0, #0x02, reg1_error_loop
	bnec	$r1, #0x03, reg1_error_loop
	fmfdr	$r0, $fd18
	bnec	$r0, #0x04, reg1_error_loop
	bnec	$r1, #0x05, reg1_error_loop
	fmfdr	$r0, $fd19
	bnec	$r0, #0x06, reg1_error_loop
	bnec	$r1, #0x07, reg1_error_loop
	fmfdr	$r0, $fd20
	bnec	$r0, #0x08, reg1_error_loop
	bnec	$r1, #0x09, reg1_error_loop
	fmfdr	$r0, $fd21
	bnec	$r0, #0x10, reg1_error_loop
	bnec	$r1, #0x11, reg1_error_loop
	fmfdr	$r0, $fd22
	bnec	$r0, #0x12, reg1_error_loop
	bnec	$r1, #0x13, reg1_error_loop
	fmfdr	$r0, $fd23
	bnec	$r0, #0x14, reg1_error_loop
	bnec	$r1, #0x15, reg1_error_loop
	fmfdr	$r0, $fd24
	bnec	$r0, #0xFF, reg1_error_loop
	bnec	$r1, #0x01, reg1_error_loop
	fmfdr	$r0, $fd25
	bnec	$r0, #0x02, reg1_error_loop
	bnec	$r1, #0x03, reg1_error_loop
	fmfdr	$r0, $fd26
	bnec	$r0, #0x04, reg1_error_loop
	bnec	$r1, #0x05, reg1_error_loop
	fmfdr	$r0, $fd27
	bnec	$r0, #0x06, reg1_error_loop
	bnec	$r1, #0x07, reg1_error_loop
	fmfdr	$r0, $fd28
	bnec	$r0, #0x08, reg1_error_loop
	bnec	$r1, #0x09, reg1_error_loop
	fmfdr	$r0, $fd29
	bnec	$r0, #0x10, reg1_error_loop
	bnec	$r1, #0x11, reg1_error_loop
	fmfdr	$r0, $fd30
	bnec	$r0, #0x12, reg1_error_loop
	bnec	$r1, #0x13, reg1_error_loop
	fmfdr	$r0, $fd31
	bnec	$r0, #0x14, reg1_error_loop
	bnec	$r1, #0x15, reg1_error_loop
	#endif
#endif

	/* Restore the registers clobbered by the test. */
	popm	$r0, $r1

	/* Everything passed, increment the loop counter. */
	pushm   $r0, $r1
	la      $r0, ullRegTest1LoopCounter
	lwi     $r1, [$r0]
	addi	$r1, $r1, 1
	swi     $r1, [$r0]
	popm    $r0, $r1

	/* Start again. */
	b       reg1_loop

reg1_error_loop:
	/* If this line is hit then there was an error in a core register value.
	The loop ensures the loop counter stops incrementing. */
	b       reg1_error_loop
	nop

/*-----------------------------------------------------------*/

.type vRegTest2Implementation, %function
vRegTest2Implementation:

	/* Fill each general purpose register with a known value. */
	move	$r0, #0x1ff
	move	$r1, #0x010
	move	$r2, #0x020
	move	$r3, #0x030
	move	$r4, #0x040
	move	$r5, #0x050
	move	$r6, #0x060
	move	$r7, #0x070
	move	$r8, #0x080
	move	$r9, #0x090
	move	$r10, #0x100
#ifdef __NDS32_REDUCE_REGS__
	move	$r15, #0x150
#else
	move	$r11, #0x110
	move	$r12, #0x120
	move	$r13, #0x130
	move	$r14, #0x140
	move	$r15, #0x150
	move	$r16, #0x160
	move	$r17, #0x170
	move	$r18, #0x180
	move	$r19, #0x190
	move	$r20, #0x200
	move	$r21, #0x210
	move	$r22, #0x220
	move	$r23, #0x230
	move	$r24, #0x240
	move	$r25, #0x250
	/* $26/$27 ($p0/$p1) is unused */
#endif
	move	$r28, #0x280
	/* $29 ($gp) is the Global pointer  - don't mess with $29. */
	move	$r30, #0x300

	/* Fill each FPU register with a known value. */
#if ( configSUPPORT_FPU == 1 )
	fmtdr	$r0, $fd0
	fmtdr	$r2, $fd1
	fmtdr	$r4, $fd2
	fmtdr	$r6, $fd3
	#if ( portFPU_REGS > 8 )
	fmtdr	$r8, $fd4
	fmtdr	$r10, $fd5
	fmtdr	$r12, $fd6
	fmtdr	$r14, $fd7
	#endif
	#if ( portFPU_REGS > 16 )
	fmtdr	$r0, $fd8
	fmtdr	$r2, $fd9
	fmtdr	$r4, $fd10
	fmtdr	$r6, $fd11
	fmtdr	$r8, $fd12
	fmtdr	$r10, $fd13
	fmtdr	$r12, $fd14
	fmtdr	$r14, $fd15
	#endif
	#if ( portFPU_REGS > 32 )
	fmtdr	$r0, $fd16
	fmtdr	$r2, $fd17
	fmtdr	$r4, $fd18
	fmtdr	$r6, $fd19
	fmtdr	$r8, $fd20
	fmtdr	$r10, $fd21
	fmtdr	$r12, $fd22
	fmtdr	$r14, $fd23
	fmtdr	$r0, $fd24
	fmtdr	$r2, $fd25
	fmtdr	$r4, $fd26
	fmtdr	$r6, $fd27
	fmtdr	$r8, $fd28
	fmtdr	$r10, $fd29
	fmtdr	$r12, $fd30
	fmtdr	$r14, $fd31
	#endif
#endif

	/* Loop, checking each itteration that each register still contains the
	expected value. */
reg2_loop:

	/* Test each general purpose register to check that it still contains the
	expected known value, jumping to reg2_error_loop if any register contains
	an unexpected value. */
	bnec	$r0, #0x1FF, reg2_error_loop
	bnec	$r1, #0x010, reg2_error_loop
	bnec	$r2, #0x020, reg2_error_loop
	bnec	$r3, #0x030, reg2_error_loop
	bnec	$r4, #0x040, reg2_error_loop
	bnec	$r5, #0x050, reg2_error_loop
	bnec	$r6, #0x060, reg2_error_loop
	bnec	$r7, #0x070, reg2_error_loop
	bnec	$r8, #0x080, reg2_error_loop
	bnec	$r9, #0x090, reg2_error_loop
	bnec	$r10, #0x100, reg2_error_loop
#ifdef __NDS32_REDUCE_REGS__
	bnec	$r15, #0x150, reg2_error_loop
#else
	bnec	$r11, #0x110, reg2_error_loop
	bnec	$r12, #0x120, reg2_error_loop
	bnec	$r13, #0x130, reg2_error_loop
	bnec	$r14, #0x140, reg2_error_loop
	bnec	$r15, #0x150, reg2_error_loop
	bnec	$r16, #0x160, reg2_error_loop
	bnec	$r17, #0x170, reg2_error_loop
	bnec	$r18, #0x180, reg2_error_loop
	bnec	$r19, #0x190, reg2_error_loop
	bnec	$r20, #0x200, reg2_error_loop
	bnec	$r21, #0x210, reg2_error_loop
	bnec	$r22, #0x220, reg2_error_loop
	bnec	$r23, #0x230, reg2_error_loop
	bnec	$r24, #0x240, reg2_error_loop
	bnec	$r25, #0x250, reg2_error_loop
#endif
	bnec	$r28, #0x280, reg2_error_loop
	bnec	$r30, #0x300, reg2_error_loop

	/* Check every floating point register to ensure it contains the expected
	value.  First save the registers clobbered by the test. */
	pushm	$r0, $r1
#if ( configSUPPORT_FPU == 1 )
	fmfdr	$r0, $fd0
	bnec	$r0, #0x1FF, reg2_error_loop
	bnec	$r1, #0x010, reg2_error_loop
	fmfdr	$r0, $fd1
	bnec	$r0, #0x020, reg2_error_loop
	bnec	$r1, #0x030, reg2_error_loop
	fmfdr	$r0, $fd2
	bnec	$r0, #0x040, reg2_error_loop
	bnec	$r1, #0x050, reg2_error_loop
	fmfdr	$r0, $fd3
	bnec	$r0, #0x060, reg2_error_loop
	bnec	$r1, #0x070, reg2_error_loop
	#if ( portFPU_REGS > 8 )
	fmfdr	$r0, $fd4
	bnec	$r0, #0x080, reg2_error_loop
	bnec	$r1, #0x090, reg2_error_loop
	fmfdr	$r0, $fd5
	bnec	$r0, #0x100, reg2_error_loop
	bnec	$r1, #0x110, reg2_error_loop
	fmfdr	$r0, $fd6
	bnec	$r0, #0x120, reg2_error_loop
	bnec	$r1, #0x130, reg2_error_loop
	fmfdr	$r0, $fd7
	bnec	$r0, #0x140, reg2_error_loop
	bnec	$r1, #0x150, reg2_error_loop
	#endif
	#if ( portFPU_REGS > 16 )
	fmfdr	$r0, $fd8
	bnec	$r0, #0x1FF, reg2_error_loop
	bnec	$r1, #0x010, reg2_error_loop
	fmfdr	$r0, $fd9
	bnec	$r0, #0x020, reg2_error_loop
	bnec	$r1, #0x030, reg2_error_loop
	fmfdr	$r0, $fd10
	bnec	$r0, #0x040, reg2_error_loop
	bnec	$r1, #0x050, reg2_error_loop
	fmfdr	$r0, $fd11
	bnec	$r0, #0x060, reg2_error_loop
	bnec	$r1, #0x070, reg2_error_loop
	fmfdr	$r0, $fd12
	bnec	$r0, #0x080, reg2_error_loop
	bnec	$r1, #0x090, reg2_error_loop
	fmfdr	$r0, $fd13
	bnec	$r0, #0x100, reg2_error_loop
	bnec	$r1, #0x110, reg2_error_loop
	fmfdr	$r0, $fd14
	bnec	$r0, #0x120, reg2_error_loop
	bnec	$r1, #0x130, reg2_error_loop
	fmfdr	$r0, $fd15
	bnec	$r0, #0x140, reg2_error_loop
	bnec	$r1, #0x150, reg2_error_loop
	#endif
	#if ( portFPU_REGS > 32 )
	fmfdr	$r0, $fd16
	bnec	$r0, #0x1FF, reg2_error_loop
	bnec	$r1, #0x010, reg2_error_loop
	fmfdr	$r0, $fd17
	bnec	$r0, #0x020, reg2_error_loop
	bnec	$r1, #0x030, reg2_error_loop
	fmfdr	$r0, $fd18
	bnec	$r0, #0x040, reg2_error_loop
	bnec	$r1, #0x050, reg2_error_loop
	fmfdr	$r0, $fd19
	bnec	$r0, #0x060, reg2_error_loop
	bnec	$r1, #0x070, reg2_error_loop
	fmfdr	$r0, $fd20
	bnec	$r0, #0x080, reg2_error_loop
	bnec	$r1, #0x090, reg2_error_loop
	fmfdr	$r0, $fd21
	bnec	$r0, #0x100, reg2_error_loop
	bnec	$r1, #0x110, reg2_error_loop
	fmfdr	$r0, $fd22
	bnec	$r0, #0x120, reg2_error_loop
	bnec	$r1, #0x130, reg2_error_loop
	fmfdr	$r0, $fd23
	bnec	$r0, #0x140, reg2_error_loop
	bnec	$r1, #0x150, reg2_error_loop
	fmfdr	$r0, $fd24
	bnec	$r0, #0x1FF, reg2_error_loop
	bnec	$r1, #0x010, reg2_error_loop
	fmfdr	$r0, $fd25
	bnec	$r0, #0x020, reg2_error_loop
	bnec	$r1, #0x030, reg2_error_loop
	fmfdr	$r0, $fd26
	bnec	$r0, #0x040, reg2_error_loop
	bnec	$r1, #0x050, reg2_error_loop
	fmfdr	$r0, $fd27
	bnec	$r0, #0x060, reg2_error_loop
	bnec	$r1, #0x070, reg2_error_loop
	fmfdr	$r0, $fd28
	bnec	$r0, #0x080, reg2_error_loop
	bnec	$r1, #0x090, reg2_error_loop
	fmfdr	$r0, $fd29
	bnec	$r0, #0x100, reg2_error_loop
	bnec	$r1, #0x110, reg2_error_loop
	fmfdr	$r0, $fd30
	bnec	$r0, #0x120, reg2_error_loop
	bnec	$r1, #0x130, reg2_error_loop
	fmfdr	$r0, $fd31
	bnec	$r0, #0x140, reg2_error_loop
	bnec	$r1, #0x150, reg2_error_loop
	#endif
#endif

	/* Restore the registers clobbered by the test. */
	popm	$r0, $r1

	/* Everything passed, increment the loop counter. */
	pushm	$r0, $r1
	la      $r0, ullRegTest2LoopCounter
	lwi     $r1, [$r0]
	addi    $r1, $r1, 1
	swi     $r1, [$r0]
	popm    $r0, $r1

	/* Start again. */
	b       reg2_loop

reg2_error_loop:
	/* If this line is hit then there was an error in a core register value.
	The loop ensures the loop counter stops incrementing. */
	b       reg2_error_loop
	nop

	.end

