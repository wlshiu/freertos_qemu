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

/* Includes ------------------------------------------------------------------*/
#include "stm32f4_discovery.h"


/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
void ToggleLED1_Task(void*);
void ToggleLED2_Task(void*);

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

    /* LED configurations:
     * LD4: GPIO PD12 (0: OFF, 1: ON)
     * LD3: GPIO PD13 (0: OFF, 1: ON)
     * LD5: GPIO PD14 (0: OFF, 1: ON)
     * LD6: GPIO PD15 (0: OFF, 1: ON)
     */
    BSP_LED_Init(LED4);
    BSP_LED_Init(LED3);
    BSP_LED_Init(LED5);
    BSP_LED_Init(LED6);

    /* Create tasks */
    xTaskCreate(
        ToggleLED1_Task,                  /* Function pointer */
        "Task_LED1",                      /* Task name - for debugging only*/
        configMINIMAL_STACK_SIZE,         /* Stack depth in words */
        (void*) NULL,                     /* Pointer to tasks arguments (parameter) */
        tskIDLE_PRIORITY + 3UL,           /* Task priority*/
        NULL                              /* Task handle */
    );

    xTaskCreate(
        ToggleLED2_Task,                  /* Function pointer */
        "Task_LED2",                      /* Task name - for debugging only*/
        configMINIMAL_STACK_SIZE,         /* Stack depth in words */
        (void*) NULL,                     /* Pointer to tasks arguments (parameter) */
        tskIDLE_PRIORITY + 2UL,           /* Task priority*/
        NULL                              /* Task handle */
    );

    /* Start the scheduler. */
    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following line
    will never be reached.  If the following line does execute, then there was
    insufficient FreeRTOS heap memory available for the idle and/or timer tasks
    to be created.  See the memory management section on the FreeRTOS web site
    for more details. */
    for( ;; );

}


/**
 * ToggleLED1_Task: Toggle LED1 via RTOS Timer
 */
void ToggleLED1_Task(void *pvParameters)
{
    while (1)
    {
        BSP_LED_On(LED4); // LED4 ON
        vTaskDelay(500 / portTICK_RATE_MS);

        BSP_LED_On(LED3); // LED3 ON
        vTaskDelay(500 / portTICK_RATE_MS);

        BSP_LED_Off(LED3);
        vTaskDelay(500 / portTICK_RATE_MS);

        BSP_LED_Off(LED4);
        vTaskDelay(500 / portTICK_RATE_MS);
    }
}

/**
 * ToggleLED2_Task: Toggle LED2 via RTOS Timer
 */
void ToggleLED2_Task(void *pvParameters)
{
    while (1)
    {
        BSP_LED_On(LED5); // LED5 ON
        vTaskDelay(500 / portTICK_RATE_MS);

        BSP_LED_On(LED6); // LED6 ON
        vTaskDelay(500 / portTICK_RATE_MS);

        BSP_LED_Off(LED5);
        vTaskDelay(500 / portTICK_RATE_MS);

        BSP_LED_Off(LED6);
        vTaskDelay(500 / portTICK_RATE_MS);
    }
}


/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
    /* vApplicationMallocFailedHook() will only be called if
    configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
    function that will get called if a call to pvPortMalloc() fails.
    pvPortMalloc() is called internally by the kernel whenever a task, queue,
    timer or semaphore is created.  It is also called by various parts of the
    demo application.  If heap_1.c or heap_2.c are used, then the size of the
    heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
    FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
    to query the size of free heap space that remains (although it does not
    provide information on how the remaining heap might be fragmented). */
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
    to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
    task.  It is essential that code added to this hook function never attempts
    to block in any way (for example, call xQueueReceive() with a block time
    specified, or call vTaskDelay()).  If the application makes use of the
    vTaskDelete() API function (as this demo application does) then it is also
    important that vApplicationIdleHook() is permitted to return to its calling
    function, because it is the responsibility of the idle task to clean up
    memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
/*-----------------------------------------------------------*/
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
