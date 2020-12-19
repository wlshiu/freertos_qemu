/*
 * FreeRTOS Kernel V10.1.1
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software. If you wish to use our Amazon
 * FreeRTOS name, please do so in a fair use way that does not cause confusion.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */


#ifndef PORTMACRO_H
#define PORTMACRO_H

#ifdef __cplusplus
extern "C" {
#endif

/* System Includes. */
#include <nds32_intrinsic.h>

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */
#define portCHAR        char
#define portFLOAT       float
#define portDOUBLE      double
#define portLONG        long
#define portSHORT       short
#define portSTACK_TYPE  uint32_t
#define portBASE_TYPE   long

typedef portSTACK_TYPE StackType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;

#if( configUSE_16_BIT_TICKS == 1 )
    typedef uint16_t TickType_t;
    #define portMAX_DELAY ( TickType_t )    0xffff
#else
    typedef uint32_t TickType_t;
    #define portMAX_DELAY ( TickType_t )    0xffffffffUL

    /* 32-bit tick type on a 32-bit architecture, so reads of the tick count do
    not need to be guarded with a critical section. */
    #define portTICK_TYPE_IS_ATOMIC 1
#endif
/*-----------------------------------------------------------*/

/* MPU specific constants. */
#define portUSING_MPU_WRAPPERS          1
#define portPRIVILEGE_BIT               ( 0x80000000UL )

#define portMPU_REGION_READ_WRITE               ( 0x03UL << 24UL )
#define portMPU_REGION_PRIVILEGED_READ_ONLY     ( 0x05UL << 24UL )
#define portMPU_REGION_READ_ONLY                ( 0x06UL << 24UL )
#define portMPU_REGION_PRIVILEGED_READ_WRITE    ( 0x01UL << 24UL )
#define portMPU_REGION_CACHEABLE_BUFFERABLE     ( 0x07UL << 16UL )
#define portMPU_REGION_EXECUTE_NEVER            ( 0x01 << 28 )

#define portFIRST_CONFIGURABLE_REGION       ( 3 )
#define portLAST_CONFIGURABLE_REGION        ( 7 )
#define portNUM_CONFIGURABLE_REGIONS        ( 3 )
#define portTOTAL_NUM_REGIONS               ( portNUM_CONFIGURABLE_REGIONS + 1 ) /* Plus one to make space for the stack region. */

#define portSWITCH_TO_USER_MODE()   __asm__ volatile ( " mfsr $r0, $PSW \n bclr $r0, $r0, #3\n mtsr $r0, $PSW\n dsb\n " :::"$r0" )

typedef struct MPU_REGION_ENTRY {
    uint32_t ulPA;
    uint32_t ulSize;
    uint32_t ulAttr;
    uint32_t ulTLB_DATA;
    uint32_t ulTLB_VPN;
} xMPU_REGION_ENTRY;

/* Plus 1 to create space for the stack region. */
typedef struct MPU_SETTINGS {
    xMPU_REGION_ENTRY xRegion[ portTOTAL_NUM_REGIONS ];
} xMPU_SETTINGS;

/* Architecture specifics. */
#define portSTACK_GROWTH            ( -1 )
#define portTICK_PERIOD_MS          ( ( TickType_t ) 1000 / configTICK_RATE_HZ )
#define portBYTE_ALIGNMENT          4096
#define portBYTE_ALIGNMENT_MASK     ( portBYTE_ALIGNMENT - 1 )

/*-----------------------------------------------------------*/

/* Scheduler utilities. */
extern void vPortYield();
#define portYIELD()                                 vPortYield();
#define portEND_SWITCHING_ISR( xSwitchRequired )    if( xSwitchRequired != pdFALSE ) __nds32__mtsr( 0x10000, NDS32_SR_INT_PEND );
#define portYIELD_FROM_ISR( x )                     portEND_SWITCHING_ISR( x )
/*-----------------------------------------------------------*/
#define portPSW_CPL_MASK            ( 0x00070000UL )

/* Critical section management. */
extern void vPortEnterCritical( void );
extern void vPortExitCritical( void );

#define portSET_INTERRUPT_MASK_FROM_ISR()       ulPortSetInterruptMask()
#define portCLEAR_INTERRUPT_MASK_FROM_ISR(x)    vPortClearInterruptMask(x)
#define portDISABLE_INTERRUPTS()                __nds32__gie_dis()
#define portENABLE_INTERRUPTS()                 __nds32__gie_en()
#define portENTER_CRITICAL()                    vPortEnterCritical()
#define portEXIT_CRITICAL()                     vPortExitCritical()

/*-----------------------------------------------------------*/

/* Task function macros as described on the FreeRTOS.org WEB site.  These are
not necessary for to use this port.  They are defined so the common demo files
(which build with all the ports) will build. */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters )  void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters )        void vFunction( void *pvParameters )
/*-----------------------------------------------------------*/

/* Tickless idle/low power functionality. */
#ifndef portSUPPRESS_TICKS_AND_SLEEP
    extern void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime );
    #define portSUPPRESS_TICKS_AND_SLEEP( xExpectedIdleTime )   vPortSuppressTicksAndSleep( xExpectedIdleTime )
#endif
/*-----------------------------------------------------------*/

/* Architecture specific optimisations. */
#ifndef configUSE_PORT_OPTIMISED_TASK_SELECTION
    #ifndef __NDS32_ISA_V3M__
        #define configUSE_PORT_OPTIMISED_TASK_SELECTION     1
    #endif
#endif

#if configUSE_PORT_OPTIMISED_TASK_SELECTION == 1

    #ifdef __NDS32_ISA_V3M__
        #error configUSE_PORT_OPTIMISED_TASK_SELECTION uses the performance extension instruction and can only be supported on V3M toolchain.
    #endif

    /* Generic helper function. */
    __attribute__( ( always_inline ) ) static inline uint8_t ucPortCountLeadingZeros( uint32_t ulBitmap )
    {
    uint8_t ucReturn;

        __asm volatile ( "clz %0, %1" : "=r" ( ucReturn ) : "r" ( ulBitmap ) );
        return ucReturn;
    }

    /* Check the configuration. */
    #if( configMAX_PRIORITIES > 32 )
        #error configUSE_PORT_OPTIMISED_TASK_SELECTION can only be set to 1 when configMAX_PRIORITIES is less than or equal to 32.  It is very rare that a system requires more than 10 to 15 difference priorities as tasks that share a priority will time slice.
    #endif

    /* Store/clear the ready priorities in a bit map. */
    #define portRECORD_READY_PRIORITY( uxPriority, uxReadyPriorities )  ( uxReadyPriorities ) |= ( 1UL << ( uxPriority ) )
    #define portRESET_READY_PRIORITY( uxPriority, uxReadyPriorities )   ( uxReadyPriorities ) &= ~( 1UL << ( uxPriority ) )

    /*-----------------------------------------------------------*/

    #define portGET_HIGHEST_PRIORITY( uxTopPriority, uxReadyPriorities )    uxTopPriority = ( 31UL - ( uint32_t ) ucPortCountLeadingZeros( ( uxReadyPriorities ) ) )

#endif /* configUSE_PORT_OPTIMISED_TASK_SELECTION */

/*-----------------------------------------------------------*/

#ifdef configASSERT
    void vPortValidateInterruptPriority( void );
    #define portASSERT_IF_INTERRUPT_PRIORITY_INVALID()      vPortValidateInterruptPriority()
#endif

#define portNOP()       __nds32__nop()

#define portINLINE      __inline

#ifndef portFORCE_INLINE
    #define portFORCE_INLINE    inline __attribute__(( always_inline))
#endif

/*-----------------------------------------------------------*/

/* Set the privilege level to user mode if xRunningPrivileged is false. */
portFORCE_INLINE static void vPortResetPrivilege( BaseType_t xRunningPrivileged )
{
    if( xRunningPrivileged != pdTRUE )
        __nds32__mtsr_dsb( __nds32__mfsr( NDS32_SR_PSW ) & ~( 1UL << 3 ), NDS32_SR_PSW );
}
/*-----------------------------------------------------------*/

portFORCE_INLINE static uint32_t ulPortSetInterruptMask( void )
{
uint32_t ulPSW = __nds32__mfsr( NDS32_SR_PSW );

#if ( configMAX_API_CALL_INTERRUPT_PRIORITY != 0 )

    __nds32__mtsr_isb( ( ulPSW & ~portPSW_CPL_MASK ) | ( configMAX_API_CALL_INTERRUPT_PRIORITY << 16 ), NDS32_SR_PSW );

    /* Return just the PSW.CPL mask bits. */
    return ( ulPSW & portPSW_CPL_MASK );
#else
    portDISABLE_INTERRUPTS();

    return ulPSW;
#endif
}
/*-----------------------------------------------------------*/

portFORCE_INLINE static void vPortClearInterruptMask( uint32_t ulNewMaskValue )
{
#if ( configMAX_API_CALL_INTERRUPT_PRIORITY != 0 )
uint32_t ulPSW = __nds32__mfsr( NDS32_SR_PSW );

    __nds32__mtsr( ( ulPSW & ~portPSW_CPL_MASK ) | ulNewMaskValue, NDS32_SR_PSW );
#else
    if ( ulNewMaskValue & 1 )
        portENABLE_INTERRUPTS();
#endif
}
/*-----------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif /* PORTMACRO_H */

