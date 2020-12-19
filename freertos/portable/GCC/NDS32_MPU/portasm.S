/*!******************************************************************************************************** */
/*!                                               FreeRTOS                                                  */
/*!                                                                                                         */
/*!                               (c) Copyright 2018, Andes Techonology                                     */
/*!                                          All Rights Reserved                                            */
/*!                                                                                                         */
/*!                                           NDS32 Generic Port                                            */
/*!                                            GNU C Compiler                                               */
/*!                                                                                                         */
/*!******************************************************************************************************** */

/*!******************************************************************************************************** */
/*! INCLUDE ASSEMBLY CONSTANTS                                                                              */
/*!******************************************************************************************************** */

#include "FreeRTOSConfig.h"
#include "ISR_Support.h"

/*!******************************************************************************************************** */
/*! IMPORT and EXPORT                                                                                       */
/*!******************************************************************************************************** */

    .global vPortStartFirstTask
    .global FreeRTOS_SWI_Handler

    .section privileged_functions

/*!************************************************************************* */
/*!                                                                          */
/*!   PortMacro Function, Call by C function                                 */
/*!   Used by First start Scheduler                                          */
/*!   Warm up a Initalized Task                                              */
/*!                                                                          */
/*!************************************************************************* */
vPortStartFirstTask:
    /* Set first task MPU regions */
    bal vPortRestoreTaskMPU

    /* Use new pxCurrentTCB to get new stack pointer */
    la      $r0, pxCurrentTCB
    lwi     $r1, [$r0]
    lwi     $sp, [$r1]      /*! $SP = pxCurrentTCB->pxTopOfStack */

    portRESTORE_CONTEXT

    /* Return into the first task */
    iret

/******************************************************************************
 * Software interrupt is used to start the scheduler.
 *****************************************************************************/

FreeRTOS_SWI_Handler:
    portSAVE_CONTEXT

    /* Save current task's $SP into TCB */
    la      $r0, pxCurrentTCB       /*! pxCurrentTCB->pxTopOfStack = $SP */
    lwi     $r1, [$r0]
    swi     $sp, [$r1]

    /* Clear SWI pending */
    movi    $r0, 0x0
    mtsr    $r0, $INT_PEND          /*! clean SWI pending */

    bal vTaskSwitchContext          /*! get the highest priority of Task */

    /* Since the MPU regions will be remapped when MPU setting is restored,
        we can NOT use the task stack to call MPU restore function and need to
        change to kernel stack which is mapped by VA=PA. */
    la      $sp, ucSWIStack         /*! use the VA=PA SWI stack */
    bal     vPortRestoreTaskMPU     /*! restore Task MPU regions */

    /* Use new pxCurrentTCB to get new stack pointer */
    la      $r0, pxCurrentTCB
    lwi     $r1, [$r0]
    lwi     $sp, [$r1]              /*! $SP = pxCurrentTCB->pxTopOfStack */

    portRESTORE_CONTEXT

    iret

    .section privileged_data
    .skip 128
    .align 3
ucSWIStack:

    .end
