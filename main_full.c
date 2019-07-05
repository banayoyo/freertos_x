/*
 * FreeRTOS Kernel V10.0.1
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
 * copies or substantial portions of the Software.
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

/******************************************************************************
 * NOTE:  This file only contains the source code that is specific to the
 * basic demo.  Generic functions, such FreeRTOS hook functions, are defined
 * in main.c.
 ******************************************************************************
 *
 * main_full() creates all the demo application tasks, then starts the scheduler.
 * The web documentation provides more details of the standard demo application
 * tasks, which provide no particular functionality but do provide a good
 * example of how to use the FreeRTOS API.
 *
 * In addition to the standard demo tasks, the following tasks and tests are
 * defined and/or created within this file:
 *
 * "Register tests":
 * prvRegTest1Task and prvRegTest2Task implement register tests. These functions
 * are just entry points and actual tests are written in the assembler file
 * regtest_xtensa.S. These tests populate core registers with a known set of
 * values and keep verifying them in a loop. Any corruption will indicate an
 * error in context switching mechanism.
 *
 * "Check" task:
 * This only executes every five seconds but has a high priority to ensure it
 * gets processor time. Its main function is to check that all the standard demo
 * tasks are still operational. While no errors have been discovered the check
 * task will print out "No errors", the current simulated tick time, free heap
 * size and the minimum free heap size so far. If an error is discovered in the
 * execution of a task then the check task will print out an appropriate error
 * message.
 */


/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>

/* Kernel includes. */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <timers.h>
#include <semphr.h>

/* Standard demo includes. */
#include "BlockQ.h"
#include "integer.h"
#include "semtest.h"
#include "PollQ.h"
#include "GenQTest.h"
#include "QPeek.h"
#include "recmutex.h"
#include "flop.h"
#include "TimerDemo.h"
#include "countsem.h"
#include "death.h"
#include "QueueSet.h"
#include "QueueOverwrite.h"
#include "EventGroupsDemo.h"
#include "IntSemTest.h"
#include "TaskNotify.h"
#include "QueueSetPolling.h"
#include "StaticAllocation.h"
#include "blocktim.h"
#include "AbortDelay.h"
#include "MessageBufferDemo.h"
#include "StreamBufferDemo.h"
#include "StreamBufferInterrupt.h"

/**
 * Priorities at which the tasks are created.
 */
#define mainCHECK_TASK_PRIORITY			( configMAX_PRIORITIES - 2 )
#define mainQUEUE_POLL_PRIORITY			( tskIDLE_PRIORITY + 1 )
#define mainSEM_TEST_PRIORITY			( tskIDLE_PRIORITY + 1 )
#define mainBLOCK_Q_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define mainCREATOR_TASK_PRIORITY		( tskIDLE_PRIORITY + 3 )
#define mainFLASH_TASK_PRIORITY			( tskIDLE_PRIORITY + 1 )
#define mainINTEGER_TASK_PRIORITY		( tskIDLE_PRIORITY )
#define mainGEN_QUEUE_TASK_PRIORITY		( tskIDLE_PRIORITY )
#define mainFLOP_TASK_PRIORITY			( tskIDLE_PRIORITY )
#define mainQUEUE_OVERWRITE_PRIORITY	( tskIDLE_PRIORITY )

/**
 * Period used in timer tests.
 */
#define mainTIMER_TEST_PERIOD			( 50 )

/**
 * Parameters that are passed into the register check tasks solely for the
 * purpose of ensuring parameters are passed into tasks correctly.
 */
#define mainREG_TEST_TASK_1_PARAMETER	( ( void * ) 0x12345678 )
#define mainREG_TEST_TASK_2_PARAMETER	( ( void * ) 0x87654321 )

/**
 * Determines whether to enable interrupt queue tests.
 *
 * Interrupt queue tests are used to test interrupt nesting and enabling them
 * interferes with proper functioning of other tests. This macro controls
 * whether to enable interrupt queue tests or all other tests:
 *
 * * When mainENABLE_INT_QUEUE_TESTS is set to 1, interrupt queue tests are
 *   enabled and every other test is disabled.
 * * When mainENABLE_INT_QUEUE_TESTS is set to 0, interrupt queue tests are
 *   disabled and every other test is enabled.
 */
#define mainENABLE_INT_QUEUE_TESTS		( 0 )
/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/

/**
 * The variable into which error messages are latched.
 */
static char *pcStatusMessage = "No errors";

/**
 * The following two variables are used to communicate the status of the
 * register check tasks to the check task.  If the variables keep incrementing,
 * then the register check tasks have not discovered any errors.  If a variable
 * stops incrementing, then an error has been found.
 */
volatile unsigned long ulRegTest1Counter = 0UL, ulRegTest2Counter = 0UL;

/**
 * The following variable is used to communicate whether the timers for the
 * IntQueue tests have been Initialized. This is needed to ensure that the queues
 * are accessed from the tick hook only after they have been created in the
 * interrupt queue test.
 */
volatile BaseType_t xTimerForQueueTestInitialized = pdFALSE;
/*-----------------------------------------------------------*/

int main_full( void )
{
	/* Start the check task as described at the top of this file. */

	#if( mainENABLE_INT_QUEUE_TESTS == 0 )
	{
		#if( configUSE_PREEMPTION != 0  )
		{
			/* Don't expect these tasks to pass when preemption is not used. */
			//vStartTimerDemoTask( mainTIMER_TEST_PERIOD );
		}
		#endif
	}
	#else /* mainENABLE_INT_QUEUE_TESTS */
	{
		/* Start interrupt queue test tasks. */
		//vStartInterruptQueueTasks();
	}
	#endif /* mainENABLE_INT_QUEUE_TESTS */

	/* Start the scheduler itself. */
	vTaskStartScheduler();

	/* Should never get here unless there was not enough heap space to create
	 * the idle and other system tasks. */
	return 0;
}
/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/
