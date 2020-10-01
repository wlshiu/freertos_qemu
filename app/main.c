
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"


#define STACK_SIZE  1024
#define ITERATIONS  50

extern int printf(const char* s, ...);

/* Task to be created. */
void vTaskTest( void * pvParameters )
{
    int i;

    /* The parameter value is expected to be 1 as 1 is passed in the
    pvParameters value in the call to xTaskCreate() below. */
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );

    for(i = 0; i < ITERATIONS; i++)
    {
        vTaskDelay(1);
        printf(".");
    }

    vTaskDelete(NULL);
}

void vTaskTest2( void * pvParameters )
{
    int i;

    /* The parameter value is expected to be 1 as 1 is passed in the
    pvParameters value in the call to xTaskCreate() below. */
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );

    for(i = 0; i < ITERATIONS; i++)
    {
        vTaskDelay(2);
        printf("+");
    }

    vTaskDelete(NULL);
}

/* Function that creates a task. */
int main( void )
{
    BaseType_t xReturned;
    TaskHandle_t xHandle = NULL;
    TaskHandle_t xHandle2 = NULL;

    printf("Create task\n");

    do {
        /* Create the task, storing the handle. */
        xReturned = xTaskCreate(
                        vTaskTest,       /* Function that implements the task. */
                        "Test1",          /* Text name for the task. */
                        STACK_SIZE,      /* Stack size in words, not bytes. */
                        ( void * ) 1,    /* Parameter passed into the task. */
                        tskIDLE_PRIORITY,/* Priority at which the task is created. */
                        &xHandle );      /* Used to pass out the created task's handle. */

        if( xReturned != pdPASS )
        {
            printf("Task create fail\n");
            break;
        }

        xReturned = xTaskCreate(
                        vTaskTest2,      /* Function that implements the task. */
                        "Test2",          /* Text name for the task. */
                        STACK_SIZE,      /* Stack size in words, not bytes. */
                        ( void * ) 1,    /* Parameter passed into the task. */
                        tskIDLE_PRIORITY,/* Priority at which the task is created. */
                        &xHandle2 );     /* Used to pass out the created task's handle. */

        if( xReturned != pdPASS )
        {
            printf("Task create fail\n");
            break;
        }

        vTaskStartScheduler();

    } while(0);

    while(1) {}

    /* Should never get here! */
    return 0;
}

#if configUSE_TICK_HOOK
void vApplicationTickHook( void )
{
}
#endif

#if configUSE_MALLOC_FAILED_HOOK
void vApplicationMallocFailedHook ( void )
{
    printf("\nMalloc fail, stopping.");
    while(1) {}
}
#endif

#if configCHECK_FOR_STACK_OVERFLOW
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    printf("\n%s: Stack overflow, stopping.", pcTaskName);
    while(1) {}
}
#endif
