/**
 * @file libtimer.h
 * @author S. Rohdemann, A. Krause
 * @date 2002
 * @c 2002 - Nanotron Technologies
 * @brief This file contains the type- and data definitions for the utility 
 * functions of the timer library.
 * @note The timer lib will be initialised automatically at the first call of 
 * a timer function.
 */

/*****************************************************************************
 *
 * Copyright 2002
 * Nanotron Technologies
 *
 * Author: S. Rohdemann
 *
 * Description :
 *    This file contains the type- and data definitions
 *    for the utility functions of the Timer Library.
 *
 ****************************************************************************/

#ifndef	_LIBTIMER_H
#define	_LIBTIMER_H

// #define TIMER_DEBUG 1


#ifdef	__cplusplus
extern	"C"	{
#endif	/* __cplusplus */
#include "main.h"
/**
 * @def _TIMER_NO_TIMERS
 * @brief Return value of StartTimer if no more Timers are available.
 */
#define	INVALID_TIMER		(0x80)
/**
 * @brief The handle of a timer.
 * 
 * Valid Values are < @ref INVALID_TIMER . @ref INVALID_TIMER will be the 
 * error handle.
 */
typedef	uint8_t	TimerHandleT;



/**
 * @brief The timer callback function pointer.
 *
 * This will be the function that the Timer Library calls
 * on the thread of the system main loop when a timer has expired.
 */
typedef	void	(*TimerFnT)(void *p);



/**
 * @fn TimerStart
 * @brief Creates and starts a timer.
 * @param steps Number of milliseconds until this timer expires.
 * @param callback Callback function to be called on timer expiration.
 * @param param Parameter for function @ref callback .
 * @returns Timer handle. You might want to cancel a timer before its 
 * expiration. Use the time handle and cancel its timer using function 
 * @ref TimerCancel .
 */
#ifndef TIMER_DEBUG
#define TimerStartDebug(s,c,p,a,d)	TimerStart(s,c,p)
extern	TimerHandleT	TimerStart(	uint32_t steps,
									TimerFnT callback,
									void *param);
#else
#define TimerStart(s,c,p)	TimerStartDebug(s,c,p,NULL,NULL)
extern TimerHandleT		TimerStartDebug(uint32_t steps,
									TimerFnT callback,
									void *param,
									TimerHandleT *appHandle,
									char *debugString);
#endif /*TIMER_DEBUG*/



/**
 * @brief Cancels a currently running timer.
 * @param handle Handle of the timer to be cancelled.
 * @returns TRUE on success, FALSE otherwise.
 * @note Make sure that the timer actually running. Make sure to invalidate 
 * your timer handles on expiration or after being cancelled.
 */
#ifndef TIMER_DEBUG
extern	bool	TimerCancel(TimerHandleT handle);
#else
#define TimerCancel(h)			TimerCancelDebug(h,&(h))
extern	bool	TimerCancelDebug(TimerHandleT handle, TimerHandleT *appHandle);
#endif /*TIMER_DEBUG*/


/**
 * @brief Returns the time until the next timer event expires.
 * @returns Time until the next timer event expires, 0 if no timer is running.
 * 
 * This is a utility function, e.g. for power saving features.
 */
extern uint32_t TimerTimeUntilNextEvent(void);



/**
 * @brief Manages timers during normal operation. 
 * @note This function should be polled in short cycles, at least once per 
 * millisecond.
 */
extern	void	TimerPoll(void);



#ifdef TIMER_DEBUG
/**
 * @brief Returns the maximum number of timers used simultaneously.
 * @returns Maximum number of timers used simultaneously.
 * 
 * This function is for debugging purposes, only. 
 */
uint8_t TimerMaxNumTimersUsed(void);
#endif /*TIMER_DEBUG*/


#ifdef	__cplusplus
}
#endif	/* __cplusplus */

#endif	/* _LIBTIMER_H */
