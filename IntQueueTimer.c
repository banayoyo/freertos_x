/*
 * FreeRTOS Kernel V10.0.0
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Demo includes. */
#include "IntQueueTimer.h"
#include "IntQueue.h"

/* Xtensa includes. */
#include <xtensa/corebits.h>
#include <xtensa/config/system.h>
#include <xtensa_api.h>
#include <xtensa/hal.h>
/*-----------------------------------------------------------*/

/* Check if Timer1 is available. */
#if XCHAL_TIMER1_INTERRUPT != XTHAL_TIMER_UNCONFIGURED
	#if XCHAL_INT_LEVEL( XCHAL_TIMER1_INTERRUPT ) <= XCHAL_EXCM_LEVEL
		#define SECOND_TIMER_AVAILABLE		1
	#endif
#endif

#ifndef SECOND_TIMER_AVAILABLE
	#define SECOND_TIMER_AVAILABLE			0
#endif

/**
 * Timer0 is used to drive systick and therefore we use Timer1
 * as second interrupt which runs on a higher priority than
 * Timer0. This ensures that systick will get interrupted by
 * this timer and hence we can test interrupt nesting.
 */
#define SECOND_TIMER_INDEX					1

/**
 * Frequency of the second timer - This timer is configured at
 * a frequency offset of 17 from the systick timer.
 */
#define SECOND_TIMER_TICK_RATE_HZ			( configTICK_RATE_HZ + 17 )
#define SECOND_TIMER_TICK_DIVISOR			( configCPU_CLOCK_HZ / SECOND_TIMER_TICK_RATE_HZ )
/*-----------------------------------------------------------*/

/* Defined in main_full.c. */
extern BaseType_t xTimerForQueueTestInitialized;
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/
