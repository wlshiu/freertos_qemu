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

/* FreeRTOS includes. */
#include "FreeRTOS.h"

/* Platform includes. */

/*-----------------------------------------------------------*/

/*
 * The application must provide a function that configures a peripheral to
 * create the FreeRTOS tick interrupt, then define configSETUP_TICK_INTERRUPT()
 * in FreeRTOSConfig.h to call the function. This file contains a function
 * that is suitable for use on the Andes AE210P.
 */
void vConfigureTickInterrupt( void )
{
    /* Set PIT priority level to the lowest possible. */
    __nds32__mtsr(__nds32__mfsr( NDS32_SR_INT_PRI ) | (3 << ( ( IRQ_PIT_VECTOR ) << 1 ) ), NDS32_SR_INT_PRI );
    __nds32__mtsr(__nds32__mfsr( NDS32_SR_INT_MASK2 ) | (1 << ( IRQ_PIT_VECTOR ) ), NDS32_SR_INT_MASK2 );

    /* Enable Channel 0, 32-bit Timer with APB clock source. */
    AE3XX_PIT->CHANNEL[0].CTRL = 0x08 | 0x01;
    AE3XX_PIT->CHANNEL[0].RELOAD = ( PCLKFREQ / configTICK_RATE_HZ );
    AE3XX_PIT->INTEN |= 1;
    AE3XX_PIT->CHNEN |= 1;
}
/*-----------------------------------------------------------*/

void vClearTickInterrupt( void )
{
    AE3XX_PIT->INTST = 1;

    /* Manually clear interrupt for edge-triggered interrupt.
    Because this API is also used by task for not handling IRQ
    (for configUSE_TICKLESS_IDLE), we can't just rely on HW auto clear. */
    if( __nds32__mfsr( NDS32_SR_INT_TRIGGER ) | (1 << ( IRQ_PIT_VECTOR )) )
        __nds32__mtsr_isb( 1 << ( IRQ_PIT_VECTOR ), NDS32_SR_INT_PEND2 );
}

/*-----------------------------------------------------------*/


void vEnableTickInterrupt( long xEnable )
{
    if (xEnable == pdTRUE)
        AE3XX_PIT->CHANNEL[0].CTRL |= 1;
    else
        AE3XX_PIT->CHANNEL[0].CTRL &= ~1;
}
/*-----------------------------------------------------------*/

void vSetTickReload( uint32_t ulReload )
{
    AE3XX_PIT->CHANNEL[0].RELOAD = ulReload;
}
/*-----------------------------------------------------------*/

uint32_t ulGetTickCounter( void )
{
    return AE3XX_PIT->CHANNEL[0].COUNTER;
}
