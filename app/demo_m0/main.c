/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    19-September-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */


#include <stdio.h>
#include "trace.h"
#include "stm32f0xx_hal.h"

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    /* Update SystemCoreClock variable according to Clock Register Values. The tick frequency is
     * set by the scheduler.
     */
    SystemCoreClockUpdate();

    /* Most systems default to the wanted configuration, with the noticeable exception of the STM32
     * driver library. If you are using an STM32 with the STM32 driver library then ensure all the
     * priority bits are assigned to be preempt priority bits by calling
     * NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4) before the RTOS is started.
     */
    //HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4); // !PRIGROUP is unimplemented in QEMU ARM 2.8.0-9.2

    // GPIOD Peripheral clock enable.
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /* Init and start tracing */
    // vTraceEnable(TRC_INIT);
    // vTraceEnable(TRC_START);

    trace_puts ("Hello ARM World!");

    // Send a message to the standard output.
    puts ("1. Standard output message.");

    while(1);

}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}
#endif

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
