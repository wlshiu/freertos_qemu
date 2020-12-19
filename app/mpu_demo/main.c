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


/*
 * This file demonstrates the use of FreeRTOS-MPU.  It creates tasks in both
 * User mode and Privileged mode, and using both the original xTaskCreate() and
 * the new xTaskCreateRestricted() API functions.  The purpose of each created
 * task is documented in the comments above the task function prototype (in
 * this file), with the task behaviour demonstrated and documented within the
 * task function itself.  In addition a queue is used to demonstrate passing
 * data between protected/restricted tasks as well as passing data between an
 * interrupt and a protected/restricted task.
 */



/* Standard includes. */
#include <string.h>
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/*-----------------------------------------------------------*/

/* Misc constants. */
#define mainDONT_BLOCK					( 0 )

/* Definitions for the messages that can be sent to the check task. */
#define mainREG_TEST_1_STILL_EXECUTING	( 0 )
#define mainREG_TEST_2_STILL_EXECUTING	( 1 )
#define mainPRINT_SYSTEM_STATUS			( 2 )

/* GCC specifics. */
#define mainALIGN_TO( x )				__attribute__((aligned(x)))


/*-----------------------------------------------------------*/
/* Prototypes for functions that implement tasks. -----------*/
/*-----------------------------------------------------------*/

/*
 * Prototype for the reg test tasks.  Amongst other things, these fill the CPU
 * registers with known values before checking that the registers still contain
 * the expected values.  Each of the two tasks use different values so an error
 * in the context switch mechanism can be caught.  Both reg test tasks execute
 * at the idle priority so will get preempted regularly.  Each task repeatedly
 * sends a message on a queue so long as it remains functioning correctly.  If
 * an error is detected within the task the task is simply deleted.
 */
static void prvRegTest1Task( void *pvParameters ) __attribute__((section(".task1_text")));
static void prvRegTest2Task( void *pvParameters ) __attribute__((section(".task2_text")));

/*
 * Prototype for the check task.  The check task demonstrates various features
 * of the MPU before entering a loop where it waits for messages to arrive on a
 * queue.
 *
 * Two types of messages can be processes:
 *
 * 1) "I'm Alive" messages sent from the reg test tasks, indicating that the
 *    task is still operational.
 *
 * 2) "Print Status commands" sent periodically by the tick hook function (and
 *    therefore from within an interrupt) which command the check task to write
 *    either pass or fail to the terminal, depending on the status of the reg
 *    test tasks.
 */
static void prvCheckTask( void *pvParameters ) __attribute__((section(".task3_text")));

/*
 * Prototype for a task created in User mode using the original vTaskCreate()
 * API function.  The task demonstrates the characteristics of such a task,
 * before simply deleting itself.
 */
static void prvOldStyleUserModeTask( void *pvParameters );

/*
 * Prototype for a task created in Privileged mode using the original
 * vTaskCreate() API function.  The task demonstrates the characteristics of
 * such a task, before simply deleting itself.
 */
static void prvOldStylePrivilegedModeTask( void *pvParameters );

/*-----------------------------------------------------------*/
/* Prototypes for other misc functions.  --------------------*/
/*-----------------------------------------------------------*/

/*
 * Just configures any clocks and IO necessary.
 */
static void prvSetupHardware( void );

/*
 * Simply deletes the calling task.  The function is provided only because it
 * is simpler to call from asm code than the normal vTaskDelete() API function.
 * It has the noinline attribute because it is called from asm code.
 */
static void prvDeleteMe( void ) __attribute__((naked));

/*
 * Used by both reg test tasks to send messages to the check task.  The message
 * just lets the check task know that the task is still functioning correctly.
 * If a reg test task detects an error it will delete itself, and in so doing
 * prevent itself from sending any more 'I'm Alive' messages to the check task.
 */
static void prvSendImAlive( xQueueHandle xHandle, unsigned long ulTaskNumber );

/*
 * The check task is created with access to three memory regions (plus its
 * stack).  Each memory region is configured with different parameters and
 * prvTestMemoryRegions() demonstrates what can and cannot be accessed for each
 * region.  prvTestMemoryRegions() also demonstrates a task that was created
 * as a privileged task settings its own privilege level down to that of a user
 * task.
 */
static void prvTestMemoryRegions( void ) __attribute__((section(".task3_text")));

/*-----------------------------------------------------------*/

/* The handle of the queue used to communicate between tasks and between tasks
and interrupts.  Note that this is a file scope variable that falls outside of
any MPU region.  As such other techniques have to be used to allow the tasks
to gain access to the queue.  See the comments in the tasks themselves for
further information. */
static xQueueHandle xFileScopeCheckQueue = NULL;

#define CXMV(c,x,m,v)   (((c) << 6) | ((x) << 4) | ((m) << 1) | (v))

extern uint32_t __load_start_task1[];
extern uint32_t __load_stop_task1[];
extern uint32_t __load_start_task2[];
extern uint32_t __load_stop_task2[];
extern uint32_t __load_start_task3[];
extern uint32_t __load_stop_task3[];
extern uint32_t _task1_size[];
extern uint32_t _task2_size[];
extern uint32_t _task3_size[];

/*-----------------------------------------------------------*/
/* Data used by the 'check' task. ---------------------------*/
/*-----------------------------------------------------------*/

/* Define MPU 4KB-aligned Physical Section Base constant */
#define mainMPU_PSB_ALIGN_SIZE 4096

/* Define the constants used to allocate the check task stack.  Note that the
stack size is defined in words, not bytes. */
#define mainCHECK_TASK_STACK_SIZE_WORDS	2048

/* Declare the stack that will be used by the check task.  The kernel will
 automatically create an MPU region for the stack.  The stack alignment must
 match MPU 4KB-aligned Physical Section Base */
static portSTACK_TYPE xCheckTaskStack[ mainCHECK_TASK_STACK_SIZE_WORDS ] mainALIGN_TO( mainMPU_PSB_ALIGN_SIZE );

/* Declare three arrays - an MPU region will be created for each array
using the xTaskParameters structure below.  THIS IS JUST TO DEMONSTRATE THE
MPU FUNCTIONALITY, the data is not used by the check tasks primary function
of monitoring the reg test tasks and printing out status information.

Note that the arrays allocate slightly more RAM than is actually assigned to
the MPU region.  This is to permit writes off the end of the array to be
detected even when the arrays are placed in adjacent memory locations (with no
gaps between them).  The align size must be a power of two. */
#define mainREAD_WRITE_ARRAY_SIZE 130
#define mainREAD_WRITE_ALIGN_SIZE 128
char cReadWriteArray[ mainREAD_WRITE_ARRAY_SIZE ] mainALIGN_TO( mainMPU_PSB_ALIGN_SIZE );

#define mainREAD_ONLY_ARRAY_SIZE 260
#define mainREAD_ONLY_ALIGN_SIZE 256
char cReadOnlyArray[ mainREAD_ONLY_ARRAY_SIZE ] mainALIGN_TO( mainMPU_PSB_ALIGN_SIZE );

#define mainPRIVILEGED_ONLY_ACCESS_ARRAY_SIZE 130
#define mainPRIVILEGED_ONLY_ACCESS_ALIGN_SIZE 128
char cPrivilegedOnlyAccessArray[ mainPRIVILEGED_ONLY_ACCESS_ALIGN_SIZE ] mainALIGN_TO( mainMPU_PSB_ALIGN_SIZE );

/* Fill in a xTaskParameters structure to define the check task - this is the
structure passed to the xTaskCreateRestricted() function. */
static const xTaskParameters xCheckTaskParameters =
{
	prvCheckTask,				/* pvTaskCode - the function that implements the task. */
	( const char * ) "Check",		/* pcName			*/
	mainCHECK_TASK_STACK_SIZE_WORDS,	/* usStackDepth	- defined in words, not bytes. */
	( void * ) 0x12121212,			/* pvParameters - this value is just to test that the parameter is being passed into the task correctly. */
	( tskIDLE_PRIORITY + 1 ) | portPRIVILEGE_BIT,/* uxPriority - this is the highest priority task in the system.  The task is created in privileged mode to demonstrate accessing the privileged only data. */
	xCheckTaskStack,			/* puxStackBuffer - the array to use as the task stack, as declared above. */

	/* xRegions - In this case the xRegions array is used to create MPU regions
	for all three of the arrays declared directly above.  Each MPU region is
	created with different parameters.  Again, THIS IS JUST TO DEMONSTRATE THE
	MPU FUNCTIONALITY, the data is not used by the check tasks primary function
	of monitoring the reg test tasks and printing out status information.*/
	{
		/* Base address					Length									Parameters */
        { ( void * )__load_start_task3,	( uint32_t )_task3_size, CXMV(2,3,1,1) },
	{ cReadWriteArray,	mainREAD_WRITE_ALIGN_SIZE,		CXMV(2,0,3,1) },
        { cReadOnlyArray,	mainREAD_ONLY_ALIGN_SIZE,		CXMV(2,0,1,1) },
	}
};

/* Three MPU regions are defined for use by the 'check' task when the task is
created.  These are only used to demonstrate the MPU features and are not
actually necessary for the check task to fulfill its primary purpose.  Instead
the MPU regions are replaced with those defined by xAltRegions prior to the
check task receiving any data on the queue or printing any messages to the
debug console.  The MPU region defined below covers the GPIO peripherals used
to write to the LCD. */
static const xMemoryRegion xAltRegions[ portNUM_CONFIGURABLE_REGIONS ] =
{
	/* Base address				Length			Parameters */
	//{ mainGPIO_START_ADDRESS,	( 64 * 1024 ),	portMPU_REGION_READ_WRITE },
	{ 0,						0,				2 },
	{ 0,						0,				2 },
	{ 0,						0,				2 }
};



/*-----------------------------------------------------------*/
/* Data used by the 'reg test' tasks. -----------------------*/
/*-----------------------------------------------------------*/

/* Define the constants used to allocate the reg test task stacks.  Note that
that stack size is defined in words, not bytes. */
#define mainREG_TEST_STACK_SIZE_WORDS	2048
#define mainREG_TEST_STACK_ALIGNMENT	( mainREG_TEST_STACK_SIZE_WORDS * sizeof( portSTACK_TYPE ) )

/* Declare the stacks that will be used by the reg test tasks.  The kernel will
automatically create an MPU region for the stack.  The stack alignment must
match MPU 4KB-aligned Physical Section Base */
static portSTACK_TYPE xRegTest1Stack[ mainREG_TEST_STACK_SIZE_WORDS ] mainALIGN_TO( mainMPU_PSB_ALIGN_SIZE );
static portSTACK_TYPE xRegTest2Stack[ mainREG_TEST_STACK_SIZE_WORDS ] mainALIGN_TO( mainMPU_PSB_ALIGN_SIZE );

/* Fill in a xTaskParameters structure per reg test task to define the tasks. */
static const xTaskParameters xRegTest1Parameters =
{
	prvRegTest1Task,			/* pvTaskCode - the function that implements the task. */
	( const char * ) "RegTest1",		/* pcName		*/
	mainREG_TEST_STACK_SIZE_WORDS,		/* usStackDepth		*/
	( void * ) 0x12345678,			/* pvParameters - this value is just to test that the parameter is being passed into the task correctly. */
	tskIDLE_PRIORITY | portPRIVILEGE_BIT,	/* uxPriority - note that this task is created with privileges to demonstrate one method of passing a queue handle into the task. */
	xRegTest1Stack,				/* puxStackBuffer - the array to use as the task stack, as declared above. */
	{					/* xRegions - this task does not use any non-stack data hence all members are zero. */

		/* Base address		Length		Parameters */
		{ (void*)&__load_start_task1,	(unsigned long)&_task1_size, CXMV(2,3,1,1)},
		{ 0,				0,			0x02 },
		{ 0,				0,			0x02 },
	}
};
/*-----------------------------------------------------------*/

static xTaskParameters xRegTest2Parameters =
{
	prvRegTest2Task,			/* pvTaskCode - the function that implements the task. */
	( const char * ) "RegTest2",		/* pcName			*/
	mainREG_TEST_STACK_SIZE_WORDS,		/* usStackDepth		*/
	( void * ) NULL,			/* pvParameters	- this task uses the parameter to pass in a queue handle, but the queue is not created yet. */
	tskIDLE_PRIORITY,			/* uxPriority		*/
	xRegTest2Stack,				/* puxStackBuffer - the array to use as the task stack, as declared above. */
	{					/* xRegions - this task does not use any non-stack data hence all members are zero. */

		/* Base address		Length		Parameters */
		{ ( void * )__load_start_task2,	( uint32_t )_task2_size, CXMV(2,3,1,1)},
		{ 0,				0,			2 },
		{ 0,				0,			2 },
	}
};
/*-----------------------------------------------------------*/

static void MPU_debug_printf( const char *pcMessage )
{
	taskENTER_CRITICAL();
	printf( "%s", pcMessage );
	taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

int main( void )
{
	prvSetupHardware();

	/* Create the queue used to pass "I'm alive" messages to the check task. */
	xFileScopeCheckQueue = xQueueCreate( 1, sizeof( unsigned long ) );

	/* One check task uses the task parameter to receive the queue handle.
	This allows the file scope variable to be accessed from within the task.
	The pvParameters member of xRegTest2Parameters can only be set after the
	queue has been created so is set here. */
	xRegTest2Parameters.pvParameters = xFileScopeCheckQueue;

	/* Create the three test tasks.  Handles to the created tasks are not
	required, hence the second parameter is NULL. */
	xTaskCreateRestricted( &xRegTest1Parameters, NULL );
    xTaskCreateRestricted( &xRegTest2Parameters, NULL );
	xTaskCreateRestricted( &xCheckTaskParameters, NULL );

 	/* Create the tasks that are created using the original xTaskCreate() API function.
 	 */
	xTaskCreate(	prvOldStyleUserModeTask,	/* The function that implements the task. */
					"Task1",					/* Text name for the task. */
					1100,						/* Stack depth in words. Must large than mainMPU_PSB_ALIGN_SIZE */
					NULL,						/* Task parameters. */
					3,							/* Priority and mode (user in this case). */
					NULL						/* Handle. */
				);

	xTaskCreate(	prvOldStylePrivilegedModeTask,	/* The function that implements the task. */
					"Task2",						/* Text name for the task. */
					1100,							/* Stack depth in words. Must large than mainMPU_PSB_ALIGN_SIZE */
					NULL,							/* Task parameters. */
					( 3 | portPRIVILEGE_BIT ),		/* Priority and mode. */
					NULL							/* Handle. */
				);

	/* Start the scheduler. */
	vTaskStartScheduler();

	/* Will only get here if there was insufficient memory to create the idle
	task. */
	for( ;; );
	return 0;
}
/*-----------------------------------------------------------*/

static void prvCheckTask( void *pvParameters )
{
/* This task is created in privileged mode so can access the file scope
queue variable.  Take a stack copy of this before the task is set into user
mode.  Once that task is in user mode the file scope queue variable will no
longer be accessible but the stack copy will. */
xQueueHandle xQueue = xFileScopeCheckQueue;
long lMessage;
unsigned long ulStillAliveCounts[ 2 ] = { 0 };
const char *pcStatusMessage = "PASS\r\n";

	MPU_debug_printf( "prvCheckTask\r\n" );

	/* Just to remove compiler warning. */
	( void ) pvParameters;

	/* Demonstrate how the various memory regions can and can't be accessed.
	The task privilege is set down to user mode within this function. */
	prvTestMemoryRegions();

	/* TODO yet verification
 	  Change the memory regions allocated to this task to those initially
	  set up for demonstration purposes to those actually required by the task. */
	//vTaskAllocateMPURegions( NULL, xAltRegions );

	/* This loop performs the main function of the task, which is blocking
	on a message queue then processing each message as it arrives. */
	for( ;; )
	{
		/* Wait for the next message to arrive. */
		xQueueReceive( xQueue, &lMessage, portMAX_DELAY );

		switch( lMessage )
		{
			case mainREG_TEST_1_STILL_EXECUTING	:
					/* Message from task 1, so task 1 must still be executing. */
					( ulStillAliveCounts[ 0 ] )++;
					break;

			case mainREG_TEST_2_STILL_EXECUTING	:
					/* Message from task 2, so task 2 must still be executing. */
					( ulStillAliveCounts[ 1 ] )++;
					break;

			case mainPRINT_SYSTEM_STATUS		:
					/* Message from tick hook, time to print out the system
					status.  If messages has stopped arriving from either reg
					test task then the status must be set to fail. */
					if( ( ulStillAliveCounts[ 0 ] == 0 ) || ( ulStillAliveCounts[ 1 ] == 0 )  )
					{
						/* One or both of the test tasks are no longer sending
						'still alive' messages. */
						pcStatusMessage = "FAIL\r\n";
					}

					/* Print a pass/fail message to the Console */
					MPU_debug_printf( pcStatusMessage );

					/* Reset the count of 'still alive' messages. */
					memset( ulStillAliveCounts, 0x00, sizeof( ulStillAliveCounts ) );
					break;

		default :
					/* Something unexpected happened.  Delete this task so the
					error is apparent (no output will be displayed). */
					prvDeleteMe();
					break;
		}
	}
}
/*-----------------------------------------------------------*/

static void prvTestMemoryRegions( void )
{
long l;
char cTemp;

	/* The check task (from which this function is called) is created in the
	Privileged mode.  The privileged array can be both read from and written
	to while this task is privileged. */
	cPrivilegedOnlyAccessArray[ 0 ] = 'a';
	if( cPrivilegedOnlyAccessArray[ 0 ] != 'a' )
	{
		/* Something unexpected happened.  Delete this task so the error is
		apparent (no output will be displayed). */
		prvDeleteMe();
	}

 	/* Writing off the end of the RAM allocated to this task will *NOT* cause a
	protection fault because the task is still executing in a privileged mode.
	Uncomment the following to test. */
	/*cPrivilegedOnlyAccessArray[ mainPRIVILEGED_ONLY_ACCESS_ALIGN_SIZE ] = 'a';*/

	/* Now set the task into user mode. */
	portSWITCH_TO_USER_MODE();

	/* Accessing the privileged only array will now cause a fault.  Uncomment
	the following line to test. */
	/*cPrivilegedOnlyAccessArray[ 0 ] = 'a';*/

	/* The read/write array can still be successfully read and written. */
	char *cReadWriteArrayVA = ( char * )( 0xA0000000 );
	for( l = 0; l < mainREAD_WRITE_ALIGN_SIZE; l++ )
	{
		cReadWriteArrayVA[ l ] = 'a';
		if( cReadWriteArrayVA[ l ] != 'a' )
		{
			/* Something unexpected happened.  Delete this task so the error is
			apparent (no output will be displayed). */
			prvDeleteMe();
		}
	}

	/* But attempting to read or write off the end of the RAM allocated to this
	task will cause a fault.  Uncomment either of the following two lines to
	test. */
	/* cReadWriteArray[ 0 ] = cReadWriteArray[ -1 ]; */
	/* cReadWriteArray[ mainREAD_WRITE_ALIGN_SIZE ] = 0x00; */

	/* The read only array can be successfully read... */
	char *cReadOnlyArrayVA = ( char* )( 0xC0000000 );
	for( l = 0; l < mainREAD_ONLY_ALIGN_SIZE; l++ )
	{
		cTemp = cReadOnlyArrayVA[ l ];
	}

	/* ...but cannot be written.  Uncomment the following line to test. */
	/* cReadOnlyArray[ 0 ] = 'a'; */

	/* Writing to the first and last locations in the stack array should not
	cause a protection fault.  Note that doing this will cause the kernel to
	detect a stack overflow if configCHECK_FOR_STACK_OVERFLOW is greater than
	1. */
	portSTACK_TYPE *xCheckTaskStackVA = ( portSTACK_TYPE * )( 0x60000000 );
    xCheckTaskStackVA[ 0 ] = 0;
    xCheckTaskStackVA[ mainCHECK_TASK_STACK_SIZE_WORDS - 1 ] = 0;

	/* Writing off either end of the stack array should cause a protection
	fault, uncomment either of the following two lines to test. */
	/* xCheckTaskStack[ -1 ] = 0; */
    /* xCheckTaskStack[ mainCHECK_TASK_STACK_SIZE_WORDS ] = 0; */

	( void ) cTemp;
}
/*-----------------------------------------------------------*/

static void prvRegTest1Task( void *pvParameters )
{
/* This task is created in privileged mode so can access the file scope
queue variable.  Take a stack copy of this before the task is set into user
mode.  Once this task is in user mode the file scope queue variable will no
longer be accessible but the stack copy will. */
xQueueHandle xQueue = xFileScopeCheckQueue;

	MPU_debug_printf( "prvRegTest1Task\r\n" );

	/* Now the queue handle has been obtained the task can switch to user
	mode.  This is just one method of passing a handle into a protected
	task, the other	reg test task uses the task parameter instead. */
    portSWITCH_TO_USER_MODE();

	/* First check that the parameter value is as expected. */
	if( pvParameters != ( void * ) 0x12345678 )
	{
		/* Error detected.  Delete the task so it stops communicating with
		the check task. */
		prvDeleteMe();
	}


	for( ;; )
	{
		/* This task tests the kernel context switch mechanism by reading and
		writing directly to registers - which requires the test to be written
		in assembly code. */
		__asm volatile
		(
			"		move	$r6, #106			\n" /* Set registers to a known value.  R0 to R1 are done in the loop below. */
			"		move	$r7, #107			\n"
			"		move	$r8, #108			\n"
			"		move	$r9, #109			\n"
			"		move	$r10, #110			\n"
			"		move	$r11, #111			\n"
			"		move	$r12, #112			\n"
			"		move	$r13, #113			\n"
			"		move	$r14, #114			\n"
			"reg1loop:						\n"
			"		move	$r0, #100			\n" /* Set the scratch registers to known values - done inside the loop as they get clobbered. */
			"		move	$r1, #101			\n"
			"		move	$r2, #102			\n"
			"		move	$r3, #103			\n"
			"		move	$r4, #104			\n"
			"		move	$r5, #105			\n"
			"		bnec	$r0, #100, reg1loop_error		\n" /* Check all the registers still contain their expected values. */
			"		bnec	$r1, #101, reg1loop_error		\n"
			"		bnec	$r2, #102, reg1loop_error		\n"
			"		bnec	$r3, #103, reg1loop_error		\n"
			"		bnec	$r4, #104, reg1loop_error		\n"
			"		bnec	$r5, #105, reg1loop_error		\n"
			"		bnec	$r6, #106, reg1loop_error		\n"
			"		bnec	$r7, #107, reg1loop_error		\n"
			"		bnec	$r8, #108, reg1loop_error		\n"
			"		bnec	$r9, #109, reg1loop_error		\n"
			"		bnec	$r10, #110, reg1loop_error		\n"
			"		bnec	$r11, #111, reg1loop_error		\n"
			"		bnec	$r12, #112, reg1loop_error		\n"
			"		bnec	$r13, #113, reg1loop_error		\n"
			"		bnec	$r14, #114, reg1loop_error		\n"
			:::"$r0", "$r1", "$r2", "$r3", "$r4", "$r5", "$r6", "$r7", "$r8", "$r9", "$r10", "$r11", "$r12", "$r13", "$r14"
		);

		/* Send mainREG_TEST_1_STILL_EXECUTING to the check task to indicate that this
		task is still functioning. */
		prvSendImAlive( xQueue, mainREG_TEST_1_STILL_EXECUTING );

		/* Go back to check all the register values again. */
		__asm volatile( "		b reg1loop	" );

		/* If this line is hit then there was an error in a core register value. */
		__asm volatile
		(
			"reg1loop_error:	 		\n"
			"		bal prvDeleteMe		\n"
		);
	}
}
/*-----------------------------------------------------------*/

static void prvRegTest2Task( void *pvParameters )
{
/* The queue handle is passed in as the task parameter.  This is one method of
passing data into a protected task, the other reg test task uses a different
method. */
xQueueHandle xQueue = ( xQueueHandle ) pvParameters;

	MPU_debug_printf( "prvRegTest2Task\r\n" );

	for( ;; )
	{
		/* This task tests the kernel context switch mechanism by reading and
		writing directly to registers - which requires the test to be written
		in assembly code. */
		__asm volatile
		(
			"		move	$r6, #6			\n" /* Set registers to a known value.  R0 to R1 are done in the loop below. */
			"		move	$r7, #7			\n"
			"		move	$r8, #8			\n"
			"		move	$r9, #9			\n"
			"		move	$r10, #10			\n"
			"		move	$r11, #11			\n"
			"		move	$r12, #12			\n"
			"		move	$r13, #13			\n"
			"		move	$r14, #14			\n"
			"reg2loop:						\n"
			"		move	$r0, #0			\n" /* Set the scratch registers to known values - done inside the loop as they get clobbered. */
			"		move	$r1, #1			\n"
			"		move	$r2, #2			\n"
			"		move	$r3, #3			\n"
			"		move	$r4, #4			\n"
			"		move	$r5, #5			\n"
			"		bnec	$r0, #0, reg2loop_error		\n" /* Check all the registers still contain their expected values. */
			"		bnec	$r1, #1, reg2loop_error		\n"
			"		bnec	$r2, #2, reg2loop_error		\n"
			"		bnec	$r3, #3, reg2loop_error		\n"
			"		bnec	$r4, #4, reg2loop_error		\n"
			"		bnec	$r5, #5, reg2loop_error		\n"
			"		bnec	$r6, #6, reg2loop_error		\n"
			"		bnec	$r7, #7, reg2loop_error		\n"
			"		bnec	$r8, #8, reg2loop_error		\n"
			"		bnec	$r9, #9, reg2loop_error		\n"
			"		bnec	$r10, #10, reg2loop_error		\n"
			"		bnec	$r11, #11, reg2loop_error		\n"
			"		bnec	$r12, #12, reg2loop_error		\n"
			"		bnec	$r13, #13, reg2loop_error		\n"
			"		bnec	$r14, #14, reg2loop_error		\n"
			:::"$r0", "$r1", "$r2", "$r3", "$r4", "$r5", "$r6", "$r7", "$r8", "$r9", "$r10", "$r11", "$r12", "$r13", "$r14"
		);

		/* Send mainREG_TEST_1_STILL_EXECUTING to the check task to indicate that this
		task is still functioning. */
		prvSendImAlive( xQueue, mainREG_TEST_2_STILL_EXECUTING );

		/* Go back to check all the register values again. */
		__asm volatile( "		b reg2loop	" );

		/* If this line is hit then there was an error in a core register value. */
		__asm volatile
		(
			"reg2loop_error:	 		\n"
			"		bal prvDeleteMe		\n"
		);
	}
}
/*-----------------------------------------------------------*/

void vAssertCalled( const char * pcFile, unsigned long ulLine )
{
volatile unsigned long ul = 0;
	( void ) pcFile;
	( void ) ulLine;

	printf( "ASSERT! Line %d, file %s\r\n", ( int )ulLine, pcFile );

	taskENTER_CRITICAL();
	{
		/* Set ul to a non-zero value using the debugger to step out of this
		function. */
		while( ul == 0 )
		{
			portNOP();
		}
	}
	taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
extern unsigned long _user_common_end[];
extern unsigned long __privileged_data_start__[];
extern unsigned long __privileged_data_end__[];
volatile unsigned long *pul;
volatile unsigned long ulReadData;

	/* The idle task, and therefore this function, run in Supervisor mode and
	can therefore access all memory.  Try reading from corners of flash and
	RAM to ensure a memory fault does not occur.

	Start with the edges of the privileged data area. */
	pul = __privileged_data_start__;
	ulReadData = *pul;
	pul = __privileged_data_end__ - 1;
	ulReadData = *pul;

	/* Next the standard USER common area. */
	pul = _user_common_end - 1;
	ulReadData = *pul;

	/* Reading off the end of Flash or SRAM space should cause a fault.
	Uncomment one of the following two pairs of lines to test. */

	/* pul = _user_common_end + (mainMPU_PSB_ALIGN_SIZE >> 2);
	ulReadData = *pul; */

	( void ) ulReadData;
}
/*-----------------------------------------------------------*/

static void prvOldStyleUserModeTask( void *pvParameters )
{
extern unsigned long _user_common_end[];
volatile unsigned long *pul;
volatile unsigned long ulReadData;

/* The following lines are commented out to prevent the unused variable
compiler warnings when the tests that use the variable are also commented out.
extern unsigned long __privileged_functions_start__[];
extern unsigned long __privileged_functions_end__[];
extern unsigned long __privileged_data_start__[];
extern unsigned long __privileged_data_end__[]; */

	MPU_debug_printf( "prvOldStyleUserModeTask\r\n" );

	( void ) pvParameters;

	/* This task is created in User mode using the original xTaskCreate() API
	function.  It should have access to all Flash and RAM except that marked
	as Privileged access only.  Reading from the start and end of the non-
	privileged RAM should not cause a problem (the privileged RAM is the first
	block at the bottom of the RAM memory). */
	pul = _user_common_end - 1;
	ulReadData = *pul;

	/* Standard USER system registers are accessible. */
	ulReadData = __nds32__mfusr( NDS32_USR_IFC_LP );

	/* System registers are not accessible.  Uncomment the following line
	to test.  Also uncomment the declaration of pulSystemPeripheralRegister
	at the top of this function. */
    /* ulReadData = __nds32__mfsr( NDS32_SR_PSW ); */

	/* Reading from anywhere inside the privileged Flash or RAM should cause a
	fault.  This can be tested by uncommenting any of the following pairs of
	lines.  Also uncomment the declaration of __privileged_functions_start__
	at the top of this function. */

	/* pul = __privileged_functions_start__;
	ulReadData = *pul; */

	/* pul = __privileged_functions_end__ - 1;
	ulReadData = *pul; */

	/* pul = __privileged_data_start__;
	ulReadData = *pul; */

	/* pul = __privileged_data_end__ - 1;
	ulReadData = *pul; */

	( void ) ulReadData;

	/* Must not just run off the end of a task function, so delete this task.
	Note that because this task was created using xTaskCreate() the stack was
	allocated dynamically and I have not included any code to free it again. */
	vTaskDelete( NULL );
}
/*-----------------------------------------------------------*/


static void prvOldStylePrivilegedModeTask( void *pvParameters )
{
extern unsigned long _user_common_end[];
extern unsigned long __privileged_data_start__[];
extern unsigned long __privileged_data_end__[];
extern unsigned long __privileged_functions_start__[];
extern unsigned long __privileged_functions_end__[];
volatile unsigned long *pul;
volatile unsigned long ulReadData;

	MPU_debug_printf( "prvOldStylePrivilegedModeTask\r\n" );

	( void ) pvParameters;

	/* This task is created in Privileged mode using the original xTaskCreate()
	API	function.  It should have access to all Flash and RAM including that
	marked as Privileged access only.  So reading from the start and end of the
	non-privileged RAM should not cause a problem (the privileged RAM is the
	first block at the bottom of the RAM memory). */
	pul = __privileged_functions_end__ + 1;
	ulReadData = *pul;

	/* Likewise reading from the start and end of the non-privileged Flash
	should not be a problem (the privileged Flash is the first block at the
	bottom of the Flash memory). */
	pul = _user_common_end - 1;
	ulReadData = *pul;

	/* Reading from anywhere inside the privileged Flash or RAM should also
	not be a problem. */
	pul = __privileged_functions_start__;
	ulReadData = *pul;
	pul = __privileged_functions_end__ - 1;
	ulReadData = *pul;
	pul = __privileged_data_start__;
	ulReadData = *pul;
	pul = __privileged_data_end__ - 1;
	ulReadData = *pul;

	/* Finally, accessing both Superuser and User system registers should both be
	possible. */
    ulReadData = __nds32__mfusr( NDS32_USR_IFC_LP );
	ulReadData = __nds32__mfsr( NDS32_SR_PSW );

	( void ) ulReadData;

	/* Must not just run off the end of a task function, so delete this task.
	Note that because this task was created using xTaskCreate() the stack was
	allocated dynamically and I have not included any code to free it again. */
	vTaskDelete( NULL );
}
/*-----------------------------------------------------------*/

static void prvDeleteMe( void )
{
	vTaskDelete( NULL );
}
/*-----------------------------------------------------------*/

static void prvSendImAlive( xQueueHandle xHandle, unsigned long ulTaskNumber )
{
	if( xHandle != NULL )
	{
		xQueueSend( xHandle, &ulTaskNumber, mainDONT_BLOCK );
	}
}
/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
static unsigned long ulCallCount;
const unsigned long ulCallsBetweenSends = 5000 / portTICK_RATE_MS;
const unsigned long ulMessage = mainPRINT_SYSTEM_STATUS;
portBASE_TYPE xDummy;

	/* If configUSE_TICK_HOOK is set to 1 then this function will get called
	from each RTOS tick.  It is called from the tick interrupt and therefore
	will be executing in the privileged state. */

	ulCallCount++;

	/* Is it time to print out the pass/fail message again? */
	if( ulCallCount >= ulCallsBetweenSends )
	{
		ulCallCount = 0;

		/* Send a message to the check task to command it to check that all
		the tasks are still running then print out the status.

		This is running in an ISR so has to use the "FromISR" version of
		xQueueSend().  Because it is in an ISR it is running with privileges
		so can access xFileScopeCheckQueue directly. */
		xQueueSendFromISR( xFileScopeCheckQueue, &ulMessage, &xDummy );
	}
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName )
{
	/* If configCHECK_FOR_STACK_OVERFLOW is set to either 1 or 2 then this 
	function will automatically get called if a task overflows its stack. */
	( void ) pxTask;
	( void ) pcTaskName;
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* If configUSE_MALLOC_FAILED_HOOK is set to 1 then this function will
	be called automatically if a call to pvPortMalloc() fails.  pvPortMalloc()
	is called automatically when a task, queue or semaphore is created. */
	for( ;; );
}
/*-----------------------------------------------------------*/
