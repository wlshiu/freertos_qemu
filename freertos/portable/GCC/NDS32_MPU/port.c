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

/*-----------------------------------------------------------
 * Implementation of functions defined in portmacro.h for the NDS32 port.
 *----------------------------------------------------------*/

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "ISR_Support.h"

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#ifndef configMAX_API_CALL_INTERRUPT_PRIORITY
    #define configMAX_API_CALL_INTERRUPT_PRIORITY
    #error configMAX_API_CALL_INTERRUPT_PRIORITY must be defined.
#endif

#if( ( configMAX_API_CALL_INTERRUPT_PRIORITY > 3 ) || ( configMAX_API_CALL_INTERRUPT_PRIORITY < 0 ) )
    #error configMAX_API_CALL_INTERRUPT_PRIORITY must be between 0 and 3
#endif

#ifndef configIRQ_SWI_VECTOR
    #error configIRQ_SWI_VECTOR must be defined in FreeRTOSConfig.h to have SWI to perform context switch.
#endif

#ifndef configSETUP_TICK_INTERRUPT
    #error configSETUP_TICK_INTERRUPT() must be defined in FreeRTOSConfig.h to call the function that sets up the tick interrupt.
#endif

#ifndef configCLEAR_TICK_INTERRUPT
    #error configCLEAR_TICK_INTERRUPT must be defined in FreeRTOSConfig.h to clear which ever interrupt was used to generate the tick interrupt.
#endif

/* Constants to setup MPU region */
#define portMPU_ENTRY( N )      ( 0x20000000UL * ( N ) )

/* Constants required to access and manipulate the MPU. */
#define portMMU_CFG_MMPS_MASK       ( 3UL << 0UL )
#define portEXPECTED_MPU_TYPE_VALUE ( 1UL )

/* Constants required to set up the initial stack. */
#define portPSW_GIE         ( 1UL << 0UL )
#define portPSW_POM         ( 1UL << 3UL )
#define portPSW_IT          ( 1UL << 6UL )
#define portPSW_DT          ( 1UL << 7UL )
#define portPSW_AEN         ( 1UL << 13UL )
#define portPSW_IFCON       ( 1UL << 15UL )
#define portPSW_CPL         ( 7UL << 16UL )

#if ( configSUPPORT_ZOL == 1 )
    /* Default task PSW: enable GIE and AEN, set CPL to 7, clear IFCON */
    #define portINITIAL_PSW         ( ( __nds32__mfsr( NDS32_SR_PSW ) | portPSW_GIE | portPSW_CPL | portPSW_IT | portPSW_DT | portPSW_AEN ) & ~portPSW_IFCON )
#else
    /* Default task PSW: enable GIE, set CPL to 7, clear IFCON */
    #define portINITIAL_PSW         ( ( __nds32__mfsr( NDS32_SR_PSW ) | portPSW_GIE | portPSW_CPL | portPSW_IT | portPSW_DT) & ~portPSW_IFCON )
#endif

#define portINITIAL_PSW_UNPRIVILEGED        ( portINITIAL_PSW & ~portPSW_POM )
#define portINITIAL_PSW_PRIVILEGED          ( portINITIAL_PSW | portPSW_POM )

/* Syscall numbers for various services. */
#define portSYSCALL_RAISE_PRIVILEGE         5000

/* Each task maintains its own interrupt status in the critical nesting
variable. */
static volatile UBaseType_t     uxCriticalNesting = 0xaaaaaaaa;

/* The offset value of stack address from PA to VA. It will be used to
get stack top VA address in stack initial */
static uint32_t ulStackOffsetPAtoVA;

/*
 * Configure a number of standard MPU regions that are used by all tasks.
 */
void vPortSetupMPU( uint32_t uIOEntry ) PRIVILEGED_FUNCTION;
static uint32_t prvSetHrange( uint32_t hrange ) PRIVILEGED_FUNCTION;

/*
 * Restore the task memory region information handler
 */
void vPortRestoreTaskMPU( void ) PRIVILEGED_FUNCTION;

/*
 * Standard FreeRTOS exception handlers.
 */
#define vPortGeneralExceptionHandler trap_General_Exception

void vPortGeneralExceptionHandler( void ) __attribute__ (( naked )) PRIVILEGED_FUNCTION;

/* If the application provides an implementation of vApplicationGeneralExceptonHandler(),
 * then it will get called directly from vPortGeneralExceptionHandler() when general
 * exception occurred, except the Privileged instruction event trigged by accessing
 * PSW in xPortRaisePrivilege(). */
void vApplicationGeneralExceptonHandler( void *pxContext ) __attribute__ (( weak )) PRIVILEGED_FUNCTION;

/*
 * Standard FreeRTOS tick handlers.
 */
void FreeRTOS_Tick_Handler( void ) PRIVILEGED_FUNCTION;

/*
 * Checks to see if being called from the context of an unprivileged task, and
 * if so raises the privilege level and returns false - otherwise does nothing
 * other than return true.
 */
BaseType_t xPortRaisePrivilege( void ) __attribute__ (( naked ));

/*
 * Start first task is a separate function so it can be tested in isolation.
 */
extern void vPortStartFirstTask( void );

/*
 * Used to catch tasks that attempt to return from their implementing function.
 */
static void prvTaskExitError( void );

/*
 * Macros which used to setup the MPU regions
 */
#define CXMV( c,x,m,v )         ( ( ( c ) << 6 ) | ( ( x ) << 4) | ( ( m ) << 1 ) | ( v ) )
#define MPU_TLB( entry, hrange, psb, cxmv )            \
    do {                                               \
        __nds32__mtsr( psb | cxmv, NDS32_SR_TLB_DATA );\
        __nds32__mtsr(hrange, NDS32_SR_TLB_VPN);       \
        __nds32__dsb();                                \
        __nds32__tlbop_twr(entry);                     \
        __nds32__isb();                                \
    } while( 0 );

/*-----------------------------------------------------------*/

/*
 * Initialise the stack of a task to look exactly as if a call to
 * portSAVE_CONTEXT had been called.
 *
 * Stack Layout:
 *                High  |-----------------|
 *                      |       $R30 (LP) |
 *                      |-----------------|
 *                      |       $R29 (GP) |
 *                      |-----------------|
 *                      |       $R28 (FP) |
 *                      |-----------------|
 *                      |   $R15 | $R25   |
 *                      |-----------------|
 *                      |   $R10 | $R24   |
 *                      |-----------------|
 *                      |        .        |
 *                      |        .        |
 *                      |-----------------|
 *                      |       $R0       |
 *                      |-----------------|
 *                      |       $IFC_LP   | ( configSUPPORT_IFC )
 *                      |-----------------|
 *                      |   $LC/$LE/$LB   | ( configSUPPORT_ZOL )
 *                      |       (ZOL)     |
 *                      |-----------------|
 *                      |       $IPSW     |
 *                      |-----------------|
 *                      |       $IPC      |
 *                      |-----------------|
 *                      |    Dummy word   | ( Dummy word for 8-byte stack pointer alignment )
 *                      |-----------------|
 *                      |       $FPU      | ( configSUPPORT_FPU )
 *                      |-----------------|
 *                Low
 *
 */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters, BaseType_t xRunPrivileged )
{
    /* Simulate the stack frame as it would be created by a context switch
    interrupt. */
extern uint32_t _SDA_BASE_ __attribute__ ((weak));

    /* R0 ~ R30 registers */
    *--pxTopOfStack = ( StackType_t ) prvTaskExitError;/* R30 : $lp */
    *--pxTopOfStack = ( StackType_t ) &_SDA_BASE_;  /* R29 : $GP */
    *--pxTopOfStack = ( StackType_t ) 0x1c;     /* R28 */
#ifdef __NDS32_REDUCE_REGS__
    *--pxTopOfStack = ( StackType_t ) 0x0f;     /* R15 */
#else
    *--pxTopOfStack = ( StackType_t ) 0x19;     /* R25 */
    *--pxTopOfStack = ( StackType_t ) 0x18;     /* R24 */
    *--pxTopOfStack = ( StackType_t ) 0x17;     /* R23 */
    *--pxTopOfStack = ( StackType_t ) 0x16;     /* R22 */
    *--pxTopOfStack = ( StackType_t ) 0x15;     /* R21 */
    *--pxTopOfStack = ( StackType_t ) 0x14;     /* R20 */
    *--pxTopOfStack = ( StackType_t ) 0x13;     /* R19 */
    *--pxTopOfStack = ( StackType_t ) 0x12;     /* R18 */
    *--pxTopOfStack = ( StackType_t ) 0x11;     /* R17 */
    *--pxTopOfStack = ( StackType_t ) 0x10;     /* R16 */
    *--pxTopOfStack = ( StackType_t ) 0x0f;     /* R15 */
    *--pxTopOfStack = ( StackType_t ) 0x0e;     /* R14 */
    *--pxTopOfStack = ( StackType_t ) 0x0d;     /* R13 */
    *--pxTopOfStack = ( StackType_t ) 0x0c;     /* R12 */
    *--pxTopOfStack = ( StackType_t ) 0x0b;     /* R11 */
#endif
    *--pxTopOfStack = ( StackType_t ) 0x0a;     /* R10 */
    *--pxTopOfStack = ( StackType_t ) 0x09;         /* R9 */
    *--pxTopOfStack = ( StackType_t ) 0x08;     /* R8 */
    *--pxTopOfStack = ( StackType_t ) 0x07;     /* R7 */
    *--pxTopOfStack = ( StackType_t ) 0x06;     /* R6 */
    *--pxTopOfStack = ( StackType_t ) 0x05;     /* R5 */
    *--pxTopOfStack = ( StackType_t ) 0x04;     /* R4 */
    *--pxTopOfStack = ( StackType_t ) 0x03;     /* R3 */
    *--pxTopOfStack = ( StackType_t ) 0x02;     /* R2 */
    *--pxTopOfStack = ( StackType_t ) 0x01;     /* R1 */
    *--pxTopOfStack = ( StackType_t ) pvParameters; /* R0 : Argument */

#if ( configSUPPORT_IFC == 1 )
    /* IFC system register */
    *--pxTopOfStack = ( StackType_t ) 0x00;     /* IFC_LP */
#endif

#if ( configSUPPORT_ZOL == 1 )
    /* ZOL system registers */
    *--pxTopOfStack = ( StackType_t ) 0x00;     /* LC */
    *--pxTopOfStack = ( StackType_t ) 0x00;     /* LE */
    *--pxTopOfStack = ( StackType_t ) 0x00;     /* LB */
#endif

    /* IPSW and IPC system registers */
    if( xRunPrivileged == pdTRUE )              /* IPSW */
        *--pxTopOfStack = ( StackType_t ) portINITIAL_PSW_PRIVILEGED;
    else
        *--pxTopOfStack = ( StackType_t ) portINITIAL_PSW_UNPRIVILEGED;
    *--pxTopOfStack = ( StackType_t ) pxCode;       /* IPC : First instruction PC of task */

#if ( ( configSUPPORT_IFC == 1 ) && ( configSUPPORT_ZOL == 1 ) || ( configSUPPORT_IFC != 1 ) && ( configSUPPORT_ZOL != 1 ) )
    /* Dummy word for 8-byte stack alignment */
    *--pxTopOfStack = (portSTACK_TYPE) -1;
#endif

#if ( configSUPPORT_FPU == 1 )
    /* FPU registers */
    pxTopOfStack -= portFPU_REGS;
#endif
    //printf( "Initial Stack : 0x%x --> 0x%x\n", pxTopOfStack, ( uint32_t )pxTopOfStack + ulStackOffsetPAtoVA );

    return ( StackType_t * )( ( uint32_t )pxTopOfStack + ulStackOffsetPAtoVA );
}
/*-----------------------------------------------------------*/

void vPortGeneralExceptionHandler( void )
{
    /* General exception handler is used to raise privilege mode and return original PSW value by $r0.
     * If it is NOT from xPortRaisePrivilege() by accessing PSW, then goes back to normal general
     * exception handling. */
    __asm volatile
    (
        "   mfsr $p0, $ipc                 \n"  /* Get exception caused address */
        "   la $p1, xPortRaisePrivilege    \n"
        "   bne $p1, $p0, 1f               \n"  /* Check is it from xPortRaisePrivilege() by accessing PSW? */
        "                                  \n"  /* It is from xPortRaisePrivilege() by accessing PSW */
        "   addi $p0, $p0, #4              \n"  /* IPC + 4 */
        "   mtsr $p0, $ipc                 \n"
        "   mfsr $r0, $ipsw                \n"  /* Return IPSW value by $r0 */
        "   ori  $p0, $r0, %0              \n"  /* Change to Privilege mode before return */
        "   mtsr $p0, $IPSW                \n"
        "   iret                           \n"  /* Go back to xPortRaisePrivilege() */
        "1:                                \n"      /* Real general exception */
        "   pushm $r0, $r31                \n"  /* Save context. $r31 is dummy for stack pointer 8 bytes alignment */
        "   move $r0, $sp                  \n"
        "   bal vApplicationGeneralExceptonHandler\n"/* Call applcation exception handler */
        "die:   j die                          \n"
        :: "i" ( portPSW_POM )
    );
}
/*-----------------------------------------------------------*/

static void prvTaskExitError( void )
{
    /* A function that implements a task must not exit or attempt to return to
    its caller as there is nothing to return to.  If a task wants to exit it
    should instead call vTaskDelete( NULL ).

    Artificially force an assert() to be triggered if configASSERT() is
    defined, then stop here so application writers can catch the error. */
BaseType_t xRunningPrivileged = xPortRaisePrivilege();

    configASSERT( uxCriticalNesting == ~0UL );
    portDISABLE_INTERRUPTS();

    vPortResetPrivilege( xRunningPrivileged );
    for( ;; );
}
/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
BaseType_t xPortStartScheduler( void )
{
    /* Make Software interrupt the lowest priority interrupts. */
#if ( configIRQ_SWI_VECTOR < 16 )
    __nds32__mtsr( __nds32__mfsr( NDS32_SR_INT_PRI ) | ( 3 << ( ( configIRQ_SWI_VECTOR ) << 1 ) ), NDS32_SR_INT_PRI );
#else
    __nds32__mtsr( __nds32__mfsr( NDS32_SR_INT_PRI2 ) | ( 3 << ( ( ( configIRQ_SWI_VECTOR ) - 16 ) << 1 ) ), NDS32_SR_INT_PRI2 );
#endif
    __nds32__mtsr( __nds32__mfsr( NDS32_SR_INT_MASK2 ) | ( 1 << ( configIRQ_SWI_VECTOR ) ), NDS32_SR_INT_MASK2 );

    /* Start the timer that generates the tick ISR.  Interrupts are disabled
    here already. */
    /* Call an application function to set up the timer that will generate the
    tick interrupt.  This way the application can decide which peripheral to
    use.  A demo application is provided to show a suitable example. */
    configSETUP_TICK_INTERRUPT();

    /* Initialise the critical nesting count ready for the first task. */
    uxCriticalNesting = 0;

    /* Start the first task. */
    vPortStartFirstTask();

    /* Should not get here! */
    return 0;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
    /* Not implemented in ports where there is nothing to return to.
    Artificially force an assert. */
    configASSERT( uxCriticalNesting == 1000UL );
}
/*-----------------------------------------------------------*/

void vPortEnterCritical( void )
{
BaseType_t xRunningPrivileged = xPortRaisePrivilege();

    portDISABLE_INTERRUPTS();
    uxCriticalNesting++;

    vPortResetPrivilege( xRunningPrivileged );
}
/*-----------------------------------------------------------*/

void vPortExitCritical( void )
{
BaseType_t xRunningPrivileged = xPortRaisePrivilege();

    configASSERT( uxCriticalNesting );
    uxCriticalNesting--;
    if( uxCriticalNesting == 0 )
    {
        portENABLE_INTERRUPTS();
    }
    vPortResetPrivilege( xRunningPrivileged );
}
/*-----------------------------------------------------------*/

void vPortYield( void )
{
BaseType_t xRunningPrivileged = xPortRaisePrivilege();

    /* Pend the SWI interrupt */
    __nds32__mtsr_isb( 0x10000, NDS32_SR_INT_PEND );

    vPortResetPrivilege( xRunningPrivileged );
}
/*-----------------------------------------------------------*/

BaseType_t xPortRaisePrivilege( void )
{
    __asm volatile (
        "mfsr $r0, $PSW                 \n" /* Generate 'Privileged instruction' exception to get PSW if in user mode, and switch to privilege */
        "btst $r0, $r0, #3              \n" /* Is the task running privileged? */
        "ret                            \n"
    );

    return 0;
}
/*-----------------------------------------------------------*/

void FreeRTOS_Tick_Handler( void )
{
    /* The SysTick runs at the lowest interrupt priority, so when this interrupt
    executes all interrupts must be unmasked. */
uint32_t ulPreviousMask;

    configCLEAR_TICK_INTERRUPT();
    ulPreviousMask = portSET_INTERRUPT_MASK_FROM_ISR();
    {
        /* Increment the RTOS tick. */
        if( xTaskIncrementTick() != pdFALSE )
        {
            /* A context switch is required.  Context switching is performed in
            the SWI interrupt.  Pend the SWI interrupt. */
            __nds32__mtsr( 0x10000, NDS32_SR_INT_PEND );
        }
    }
    portCLEAR_INTERRUPT_MASK_FROM_ISR( ulPreviousMask );
}
/*-----------------------------------------------------------*/

void vApplicationGeneralExceptonHandler( void *pxContext )
{
    ( void ) pxContext;
    while( 1 );
}

static uint32_t prvSetHrange( uint32_t hrange )
{
    /* Hrange value must be 4K alignment and between 0x1000 ~ 0x20000000 range. */
    if( hrange & ~( 0x1000 - 1 ) )
    {
        if( hrange & ~( 0x20000000 - 1 ) )
            return 0x1FFFF000;      /* hrange >= 0x20000000 */
    }
    else
    {
        return 0x1000;          /* hrange < 0x1000 */
    }

    return ( ( hrange + ( 0x1000 - 1 ) ) & ( 0x1FFFF000 ) );
}
/*-----------------------------------------------------------*/

void vPortSetupMPU( uint32_t  uIOEntry )
{
extern uint32_t _stack_end[];
extern uint32_t _user_common_end[];

    /* Check the expected MPU is present. */
    if( ( __nds32__mfsr( NDS32_SR_MMU_CFG ) & portMMU_CFG_MMPS_MASK ) == portEXPECTED_MPU_TYPE_VALUE )
    {
        /* Set Fixed Fields MPU Entry ( Entry, Hrange, PSB, Attr ) */
        /* Entry 0 : Privileged / non-cache / RW ( Privilege-text/data, global bss ) */
        MPU_TLB( portMPU_ENTRY( 0 ), prvSetHrange( ( uint32_t )_stack_end ), 0x0, CXMV( 2,2,7,1 ) );

        /* Entry 1 : User / non-cache / RW ( Common code/data ) */
        MPU_TLB( portMPU_ENTRY( 1 ), prvSetHrange( ( uint32_t )_user_common_end - portMPU_ENTRY( 1 ) ), 0x05000000, CXMV( 2,3,3,1 ) );

        /* Entry I/O : User / non-cache / RW ( Peripheral ) */
        MPU_TLB( portMPU_ENTRY( uIOEntry ), prvSetHrange( 0x20000000 ), portMPU_ENTRY( uIOEntry ), CXMV( 2,3,3,1 ) );

        /* Since VA is equal to PA at this stage, we can turn on IT/DT directly */
        __nds32__mtsr_isb( __nds32__mfsr( NDS32_SR_PSW ) | ( 0x3 << 6 ), NDS32_SR_PSW );
    }
}
/*-----------------------------------------------------------*/

void vPortStoreTaskMPUSettings( xMPU_SETTINGS *xMPUSettings, const struct xMEMORY_REGION * const xRegions, StackType_t *pxBottomOfStack, uint32_t ulStackDepth )
{
extern uint32_t _tasks_start[];
int32_t lIndex;
uint32_t ul;

    if( xRegions == NULL )
    {
        /* No define gernic MPU regions, use the default setting */

        /* Task Stack Region (PA @pxBottomOfStack) : User / non-cache / RW */
        xMPUSettings->xRegion[ 0 ].ulPA = ( uint32_t ) pxBottomOfStack;
        xMPUSettings->xRegion[ 0 ].ulSize = ulStackDepth * sizeof( StackType_t );
        xMPUSettings->xRegion[ 0 ].ulAttr = CXMV( 2,3,3,1 );
        xMPUSettings->xRegion[ 0 ].ulTLB_VPN = prvSetHrange( ulStackDepth * sizeof( StackType_t ) );
        xMPUSettings->xRegion[ 0 ].ulTLB_DATA= ( ( uint32_t )pxBottomOfStack & ~0xFFF ) | CXMV( 2,3,3,1 );

        /* ulStackOffsetPAtoVA will be used by stack initialize */
        ulStackOffsetPAtoVA = portMPU_ENTRY( 3 ) - ( uint32_t ) pxBottomOfStack;

        //printf( "1 ==> pxBottomOfStack is 0x%x\r\n", pxBottomOfStack );
        //printf( "1 ==> ulStackOffsetPAtoVA is 0x%x\r\n", ulStackOffsetPAtoVA );

        /* Should FIXME : set non-restricted task allow all tasks-text read-only */
        /* TASK region (PA @_tasks_start) : User / non-cache / RO */
        xMPUSettings->xRegion[ 1 ].ulPA = ( uint32_t )_tasks_start;
        xMPUSettings->xRegion[ 1 ].ulSize = prvSetHrange( 0x20000000 );
        xMPUSettings->xRegion[ 1 ].ulAttr = CXMV( 2,3,1,1 );
        xMPUSettings->xRegion[ 1 ].ulTLB_VPN = prvSetHrange( 0x20000000 );
        xMPUSettings->xRegion[ 1 ].ulTLB_DATA= ( ( uint32_t )_tasks_start & ~0xFFF ) | CXMV( 2,3,1,1 );

        /* Invalidate all other regions. */

        for( ul = 2; ul <= portNUM_CONFIGURABLE_REGIONS; ul++ )
        {
            xMPUSettings->xRegion[ul].ulPA = 0UL;
            xMPUSettings->xRegion[ul].ulSize = 0UL;
            xMPUSettings->xRegion[ul].ulAttr = 0UL;
            xMPUSettings->xRegion[ul].ulTLB_VPN = 0UL;
            xMPUSettings->xRegion[ul].ulTLB_DATA = 0UL;
        }
    }
    else
    {

        /*
         * This function is called automatically when the task is created - in
         * which case the stack region parameters will be valid.  At all other
         * times the stack parameters will not be valid and it is assumed that the
         * stack region has already been configured.
         */
        if( ulStackDepth > 0 )
        {
            /*  Define the region that allows access to the task stack */
            xMPUSettings->xRegion[ 0 ].ulPA = ( uint32_t ) pxBottomOfStack;
            xMPUSettings->xRegion[ 0 ].ulSize = ulStackDepth * sizeof( StackType_t );
            xMPUSettings->xRegion[ 0 ].ulAttr = CXMV( 2,3,3,1 );
            xMPUSettings->xRegion[ 0 ].ulTLB_VPN = prvSetHrange( ulStackDepth * sizeof( StackType_t ) );
            xMPUSettings->xRegion[ 0 ].ulTLB_DATA= ( ( uint32_t )pxBottomOfStack & ~0xFFF ) | CXMV( 2,3,3,1 );

            ulStackOffsetPAtoVA = portMPU_ENTRY( 3 ) - (unsigned long) pxBottomOfStack;

            //printf( "2 ==> pxBottomOfStack is 0x%x\r\n", pxBottomOfStack );
            //printf( "2 ==> ulStackOffsetPAtoVA is 0x%x\r\n", ulStackOffsetPAtoVA );
        }

        lIndex = 0;

        for( ul = 1; ul <= portNUM_CONFIGURABLE_REGIONS; ul++ )
        {
            if( ( xRegions[ lIndex ] ).ulLengthInBytes > 0UL )
            {
                xMPUSettings->xRegion[ul].ulPA = ( uint32_t )xRegions[ lIndex ].pvBaseAddress;
                xMPUSettings->xRegion[ul].ulSize = xRegions[ lIndex ].ulLengthInBytes;
                xMPUSettings->xRegion[ul].ulAttr = xRegions[ lIndex ].ulParameters;
                xMPUSettings->xRegion[ul].ulTLB_VPN = prvSetHrange( xRegions[ lIndex ].ulLengthInBytes);
                xMPUSettings->xRegion[ul].ulTLB_DATA= ( ( uint32_t ) xRegions[ lIndex ].pvBaseAddress & ~0xFFF ) |
                                                      ( xRegions[ lIndex ].ulParameters );
            }
            else
            {
                /* Invalidate the region. */
                xMPUSettings->xRegion[ul].ulPA = 0UL;
                xMPUSettings->xRegion[ul].ulSize = 0UL;
                xMPUSettings->xRegion[ul].ulAttr = 0UL;
                xMPUSettings->xRegion[ul].ulTLB_VPN = 0UL;
                xMPUSettings->xRegion[ul].ulTLB_DATA = 0UL;
            }
            lIndex++;
        }
    }
}
/*-----------------------------------------------------------*/

void vPortRestoreTaskMPU( void )
{
extern void * pxCurrentTCB;
xMPU_REGION_ENTRY * p = ( xMPU_REGION_ENTRY * )( ( uint32_t )pxCurrentTCB + 4 );
int32_t i;

#ifdef CONFIG_DEBUG
    /* DO NOT MODIFY THE FOLLOWING DEBUGGING CODE; KEEP THEM AS THEY ARE */
    static int prv_seg;
    static int now_seg = -1;

    /* switch */
    prv_seg = now_seg;

    /* Get now task ovly_table index */
    now_seg = _ovly_get_idx(p[1].ulPA ,p[1].ulSize);
#endif /* CONFIG_DEBUG */

    /* Entry 2 : Task text region, VA @0x40000000 */
    MPU_TLB( portMPU_ENTRY( 2 ), p[ 1 ].ulTLB_VPN , (p[1].ulTLB_DATA & ~(0xfff)), (p[1].ulTLB_DATA & (0xfff)) );

    /* Entry 3 : Task stack region VA @0x60000000 */
    MPU_TLB( portMPU_ENTRY( 3 ), p[ 0 ].ulTLB_VPN , (p[0].ulTLB_DATA & ~(0xfff)), (p[0].ulTLB_DATA & (0xfff)) );

    for ( i = 2; i < portTOTAL_NUM_REGIONS; ++i )
    {
        if ( p[ i ].ulTLB_DATA & 1 )
        {
            MPU_TLB( portMPU_ENTRY( 3 + i ),p[i].ulTLB_VPN , (p[i].ulTLB_DATA & ~(0xfff)), (p[i].ulTLB_DATA & (0xfff)) );
        }
        else
        {
            MPU_TLB( portMPU_ENTRY( 3 + i ),p[i].ulTLB_VPN , (p[i].ulTLB_DATA & ~(0xfff)), (0x2) );
        }
    }

#ifdef CONFIG_DEBUG
    /* DO NOT MODIFY THE FOLLOWING DEBUGGING CODE; KEEP THEM AS THEY ARE */
    if ( now_seg > -1 )
    {
        /* _seg > -1 means the task under ovly section */
        /* if ( prv_seg == -1 ) the prv task is outside of ovly section, no need change mapped */
        if ( prv_seg > -1 )
        {
            _ovly_table[prv_seg].mapped = 0;
        }

        _ovly_table[now_seg].mapped = 1;
        _ovly_debug_event();
    }
#endif /* CONFIG_DEBUG */
}
/*-----------------------------------------------------------*/


#if( configASSERT_DEFINED == 1 )

    void vPortValidateInterruptPriority( void )
    {
#if ( configMAX_API_CALL_INTERRUPT_PRIORITY != 0 )
    uint8_t ucCurrentPriority;


        /* Obtain the number of the currently executing interrupt. */
        ucCurrentPriority = ( ( __nds32__mfsr( NDS32_SR_PSW ) & portPSW_CPL_MASK ) >> 16UL );

        /* The following assertion will fail if a service routine (ISR) for
        an interrupt that has been assigned a priority above
        configMAX_SYSCALL_INTERRUPT_PRIORITY calls an ISR safe FreeRTOS API
        function.  ISR safe FreeRTOS API functions must *only* be called
        from interrupts that have been assigned a priority at or below
        configMAX_SYSCALL_INTERRUPT_PRIORITY.

        Numerically low interrupt priority numbers represent logically high
        interrupt priorities, therefore the priority of the interrupt must
        be set to a value equal to or numerically *higher* than
        configMAX_SYSCALL_INTERRUPT_PRIORITY.

        FreeRTOS maintains separate thread and ISR API functions to ensure
        interrupt entry is as fast and simple as possible. */
        configASSERT( ucCurrentPriority >= configMAX_API_CALL_INTERRUPT_PRIORITY );
#endif
    }

#endif /* configASSERT_DEFINED */
