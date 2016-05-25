/**
 * @file libtimer.c
 * @author S. Rohdemann, A. Krause
 * @c Nanotron Technologies 2002-2009
 * @brief This file contains the source code for the utility functions of the
 * timer library.
 *
 */
 
#include	"liblist.h"
#include	"libtimer.h"
#include  "timer.h"

/*NOTE activate TIMER_DEBUG in libtimer.h*/
#ifdef TIMER_DEBUG
#	define TMRERR PRINTF
// #	define TMRDBG PRINTF
#	define TMRDBG NoDebugging
#	define TMRDBG_VERBOSE	1
void TimerPrint(void);
#else
#	define TMRERR NoDebugging
#	define TMRDBG NoDebugging
#	undef  TMRDBG_VERBOSE
#endif /*TIMER_DEBUG*/


typedef	struct	_ppstimert_	{
	TimerHandleT	Handle;
	uint32_t		StartValue;
	uint32_t		Expire;
	TimerFnT		Fn;
	void			*Data;
#	ifdef TIMER_DEBUG
	uint32_t		initSteps;
	uint32_t		startTime;
	uint8_t			debugId;
	TimerHandleT	*appHandleP;
	char			*debugString;
#	endif /*TIMER_DEBUG*/
}	TimerT;

/*
 * Timerpool:		CONFIG_NUM_TIMERS * 19 Bytes
 * CallbackFnList:	CONFIG_NUM_TIMERS *  4 Bytes
*/

static	TimerT			TimerPool[CONFIG_NUM_TIMERS];
static	ListT			ListPool[CONFIG_NUM_TIMERS];
static	ListT			*TimerList = NULL;
static	TimerFnT		CallBackFn[CONFIG_NUM_TIMERS];
static	void			*CallBackArg[CONFIG_NUM_TIMERS];
static	uint32_t		TimLastPoll;
static	bool			TimerLibInitalized = FALSE;
#ifdef TIMER_DEBUG
static	uint8_t			numActiveTimers = 0;
static	uint8_t			maxNumTimers = 0;
#endif /*TIMER_DEBUG*/



#ifndef TMRDBG_VERBOSE
#	define TimerPrint()
#else /*TMRDBG_VERBOSE*/
void TimerPrint(void)
{
	TimerT *timer;
	ListT *lptr;
	
	// if (maxNumTimers < CONFIG_NUM_TIMERS - 2)
	// 	return;
	
	for(lptr = TimerList; lptr != NULL; lptr = lptr->liNext)
	{
		timer = (TimerT *)(lptr->liData);

		//TMRDBG("%5ld:tmr%d: %5lu -> %5lu (%s)\n", hwclock(), timer->debugId, timer->StartValue, timer->Expire, timer->debugString);
		TMRDBG("tmr%d: %s\n", timer->debugId, timer->debugString);
	}
	TMRDBG("---\n");
}
#endif /*TMRDBG_VERBOSE*/

/****************************************************************************/
/**
 * @brief Assigns timer handles for new timers to be started.
 * @returns A valid timer handle.
 *
 * The function returns only those timer handles which were not found in the
 * timer pool at the moment (active timers as well as inactive ones). This is a
 * security mechanism to prevent timers from cancelling each other by accident
 * in an expire-expire-start-cancel-start sequence involving two concurrent
 * timers.
 */
TimerHandleT GetNewTimerHandle(void)
{
#if (CONFIG_NUM_TIMERS > INVALID_TIMER/2)
#	error (CONFIG_NUM_TIMERS >= INVALID_TIMER/2)
#	error Timer handle data type should not manage more than (INVALID_TIMER/2) timers.
#endif /*(CONFIG_NUM_TIMERS > INVALID_TIMER/2)*/

	static	TimerHandleT nextTimerHandle = 0;
	TimerHandleT maxCycles = INVALID_TIMER;
	bool	isValid = FALSE;
	TimerHandleT i;

	while (!isValid)
	{
		isValid = TRUE;

		/*increment timer handle, avoiding handles >= INVALID_TIMER*/
		if (++nextTimerHandle >= INVALID_TIMER) nextTimerHandle = 0;

		/*check all timers if the timer handle was in use recently*/
		for (i = 0; i < CONFIG_NUM_TIMERS; i++)
		{
			/*thistimer-> will also currently check inactive timers:*/
			if (((TimerPool[i].Handle) & (~INVALID_TIMER)) == nextTimerHandle)
			{
				/*this timer timer handle is in use :( */
				isValid = FALSE;
				break;
			}
		}

		if (--maxCycles == 0)
		{
			/*terminate of no timer handle is available:*/
			// TMRDBG("TH %i\n", INVALID_TIMER);
			return INVALID_TIMER;
		}
	}

	// TMRDBG("TH %i\n", nextTimerHandle);
	return nextTimerHandle;
}

/****************************************************************************/
/**
 * @brief Initialises the timer library. To be called at system startup and 
 * before any timer was started.
 * @returns TRUE on successful initialisation, FALSE otherwise.
 */
bool	TimerInitLib(void)
{
	uint16_t	i;
	ListT	*lptr;

	if(CONFIG_NUM_TIMERS == 0)
	{
		TimerLibInitalized = FALSE;
		return	FALSE;
	}

	/*reset all timers*/
	for (i = 0; i < CONFIG_NUM_TIMERS; i++)
	{
		/*initialise list entry for timer i*/
		lptr = &(ListPool[i]);
		lptr->liData = &(TimerPool[i]);
		lptr->liNext = NULL;
		lptr->liPrev = NULL;

		/*invalidate timer*/
		((TimerT*) (lptr->liData))->Handle = INVALID_TIMER;
#		ifdef TIMER_DEBUG
		((TimerT*) (lptr->liData))->debugId = (uint8_t)i;
		// ((TimerT*) (lptr->liData))->appHandleP = NULL;
#		endif /*TIMER_DEBUG*/

		CallBackFn[i] = NULL;
		CallBackArg[i] = NULL;
	}

	TimerList = NULL;
	//记录当前时间
	TimLastPoll = hwclock();
	TimerLibInitalized = TRUE;

#	ifdef TIMER_DEBUG
	numActiveTimers = 0;
	maxNumTimers = 0;
#	endif /*TIMER_DEBUG*/
	return	TRUE;
}

/****************************************************************************/
#ifdef TIMER_DEBUG
TimerHandleT	TimerStartDebug(uint32_t steps,
								TimerFnT callback,
								void *param,
								TimerHandleT *appHandle,
								char *debugString)
#else
TimerHandleT	TimerStart(	uint32_t steps,
								TimerFnT callback,
								void *param)
#endif /*TIMER_DEBUG*/
{
	ListT		*lptr, *newlptr;
	uint16_t	i;
	TimerT		*newTimer, *preceedingTimer;
	uint32_t	now = hwclock();
	uint32_t	stepsMod = steps + (now - TimLastPoll);

	// TMRDBG("+T\n");
	// TimerPrint();

	if(!TimerLibInitalized)
	{
		TimerInitLib();
	}

	/*find free timer slot*/
	//最多可以建立7个定时任务
	for(i = 0; i < CONFIG_NUM_TIMERS; i++)
	{
		newlptr = &(ListPool[i]);
		newTimer = (TimerT *) (newlptr->liData);
		ENTER_TASK;
		//查找空的链表池存放新的任务
		if(newTimer->Handle >= INVALID_TIMER)
		{
			/*free timer slot found: reserve timer handle for it:*/
			//获取新任务的标识
			newTimer->Handle = GetNewTimerHandle();
			// LEAVE_TASK;

			if (newTimer->Handle == INVALID_TIMER)
			{
				// LEAVE_TASK;
			//	TMRERR("ERROR: Could not assign timer handle!\n");
				return INVALID_TIMER;
			}

			/*store timer parameters*/
			//设置任务执行的开始时间
			newTimer->StartValue	= stepsMod;
			//设置任务失效的时间
			newTimer->Expire		= stepsMod;
			newTimer->Fn = callback;
			newTimer->Data = param;

#			ifdef TIMER_DEBUG
			newTimer->debugString = debugString;
			newTimer->appHandleP = appHandle;
			newTimer->initSteps = steps;
			newTimer->startTime = now;
			numActiveTimers++;
			if (numActiveTimers > maxNumTimers)
			{
				maxNumTimers = numActiveTimers;
			}
			if (numActiveTimers == (CONFIG_NUM_TIMERS - 1))
			{
				TMRERR("WARNING: max n/o timers active (%i)!\n", CONFIG_NUM_TIMERS);
			}
#			endif /*TIMER_DEBUG*/

			/*insert timer into list of active timers*/
			if(TimerList == NULL)
			{
				ListInsertFirst(&TimerList, newlptr);
				TimerPrint();
				return	(newTimer->Handle);
			}

			for(lptr = TimerList; lptr != NULL; lptr = lptr->liNext)
			{
				//进行着的定时器
				preceedingTimer = (TimerT *) (lptr->liData);

				if(preceedingTimer->Expire >= stepsMod)
				{
					if(lptr == TimerList)
					{
						ListInsertFirst(&TimerList, newlptr);
					}
					else
					{
						ListInsertBefore(lptr, newlptr);
					}

					TimerPrint();
					return	(newTimer->Handle);
				}

				/*Modify new timer timeout by already expired time of preceding timers:*/
				stepsMod += preceedingTimer->StartValue - preceedingTimer->Expire;
				newTimer->StartValue = stepsMod;
				newTimer->Expire = stepsMod;

				if(lptr->liNext == NULL)
				{
					ListInsertAfter(lptr, newlptr);
					TimerPrint();
					return	(newTimer->Handle);
				}
			}
		}
		LEAVE_TASK;
	}

//	TMRERR("ERROR no timers left.\n");
	return INVALID_TIMER;
}

/****************************************************************************/
#ifdef TIMER_DEBUG
bool	TimerCancelDebug(TimerHandleT handle, TimerHandleT *appHandle)
#else
bool	TimerCancel(TimerHandleT handle)
#endif /*TIMER_DEBUG*/
{
	ListT		*lptr = TimerList;
	ListT		*nextlptr;
	TimerT		*timer, *nextTimer;
	uint32_t	elapsedTime = 0;


	if(!TimerLibInitalized)
	{
		TimerInitLib();
		return FALSE;
	}

	if (handle != INVALID_TIMER)
	{
		/*search timer list for given handle*/
		while(lptr != NULL)
		{
			timer = (TimerT *) (lptr->liData);

			if(timer->Handle == handle)
			{
#				ifdef TIMER_DEBUG
				numActiveTimers--;
			
				if (timer->appHandleP != appHandle)
				{
					TMRERR("BAD APPHDL\n");
					return FALSE;
				}
#				endif /*TIMER_DEBUG*/

				elapsedTime = timer->StartValue - timer->Expire;
				nextlptr = lptr->liNext;

				if (elapsedTime > 0)
				{
					while (nextlptr != NULL)
					{
						/*update expiration timeout for subsequent timers in list*/
						nextTimer = (TimerT*)(nextlptr->liData);
						nextTimer->Expire -= elapsedTime;
						nextTimer->StartValue -= elapsedTime;
						nextlptr = nextlptr->liNext;
					}
				}

				/*invalidate timer entry*/
				ListDelete(&TimerList, lptr);
				timer->Handle |= INVALID_TIMER;

				return	TRUE;
			}

			lptr = lptr->liNext;
		}

	}
	
//	TMRERR("HDL NFND (%i)\n", (uint16_t)handle);
	return	FALSE;
}

/****************************************************************************/
void	TimerPoll(void)
{
	uint32_t	now, elapsed, firstStartValue;
	ListT		*lptr, *dptr;
	uint16_t	i, index;
	TimerT		*timer;

	if(!TimerLibInitalized)
	{
		TimerInitLib();
	}

	if(TimerList == NULL)
	{
		return;
	}

	now = hwclock();

	if (now <= TimLastPoll)
	{
		/* nothing to do, special treatment in case of hwclock overflow.*/
		TimLastPoll = now;
		return;
	}

	elapsed = now - TimLastPoll;			/* elapsed time in clocks*/
	TimLastPoll = now;
	timer = (TimerT *) (TimerList->liData);

	if(timer->Expire > elapsed)
	{
		/*not expired: decrement Expire countdown of first list element, only.*/
		timer->Expire -= elapsed;
	}
	else
	{
	
#ifdef TIMER_DEBUG
		if ((elapsed - timer->Expire) > 3) PRINTF("%ld, %s\n", timer->Expire - elapsed, timer->debugString);
#endif
		/*This timer expired.*/
		index = 0;
		if(timer->Fn != NULL)
		{
			/*add callback function to list*/
			CallBackFn[index] = timer->Fn;
			CallBackArg[index] = timer->Data;
			index++;
		}

		/*start value of this timer will be the decrement for all remaining timers*/
		firstStartValue = timer->StartValue;
		if (timer->Expire < elapsed) firstStartValue += elapsed - timer->Expire;
		/*now: (firstStartValue >= elapsed) !*/

#ifdef TIMER_DEBUG
		if (timer->initSteps != (now - timer->startTime))
		{
			// TMRDBG("tmr%i: %5li (ret %5li)\n", timer->debugId, timer->initSteps, (int32_t)(now - timer->startTime) - (int32_t)(timer->initSteps));
			// TMRDBG("T%iEx %li (%s)\n", timer->debugId, (int32_t)(now - timer->startTime) - (int32_t)(timer->initSteps), timer->debugString);
		}
		// TMRDBG("fsV=%lu\n", firstStartValue);
		TimerPrint();
		// TMRDBG("T%iEx %li (%s)\n", timer->debugId, (int32_t)(now - timer->startTime) - (int32_t)(timer->initSteps), timer->debugString);
		numActiveTimers--;
#endif	/*TIMER_DEBUG*/

		/*invalidate timer entry*/
		ListDelete(&TimerList, TimerList);
		timer->Handle |= INVALID_TIMER;

		/*any further timers to be managed?*/
		lptr = TimerList;

		while(lptr != NULL)
		{
			timer = (TimerT *) (lptr->liData);

			/*decrement all timers.*/
			if (timer->StartValue > firstStartValue)
			{
				timer->StartValue -= firstStartValue;
			}
			else
			{
				timer->StartValue = 0;
			}

			if (timer->Expire > firstStartValue)
			{
				timer->Expire -= firstStartValue;
			}
			else
			{
				timer->Expire = 0;
			}

			/*timer still active?*/
			if (timer->Expire > 0)
			{
				/*yes, go to the next timer in list.*/
				lptr = lptr->liNext;
			}
			else
			{
				/*no, this timer also expired.*/
#				ifdef TIMER_DEBUG
				// TMRDBG("+");
				if ((now - timer->startTime) != timer->initSteps)
				{
					// TMRDBG("tmr%i: %5li (ret %5li)\n", timer->debugId, timer->initSteps, (int32_t)(now - timer->startTime) - (int32_t)(timer->initSteps));
					// TMRDBG("T%iEx %li (%s)\n", timer->debugId, (int32_t)(now - timer->startTime) - (int32_t)(timer->initSteps), timer->debugString);
				}
				numActiveTimers--;
				// PRINTF("fsV=%lu\n", firstStartValue);
				TimerPrint();
				// TMRDBG("T%iEx %li (%s)\n", timer->debugId, (int32_t)(now - timer->startTime) - (int32_t)(timer->initSteps), timer->debugString);
#				endif /*TIMER_DEBUG*/

				if(timer->Fn != NULL)
				{
					/*add callback function to list*/
					CallBackFn[index] = timer->Fn;
					CallBackArg[index] = timer->Data;
					index++;
				}

				/*remove timer from list of pending timers*/
				dptr = lptr;
				lptr = lptr->liNext;
				ListDelete(&TimerList, dptr);
				/*invalidate timer entry*/
				timer->Handle |= INVALID_TIMER;
			}
		}
               //执行函数
		/*run listed callback functions*/
		for(i = 0; i < index; i++)
		{
		
			(CallBackFn[i])(CallBackArg[i]);
		}
	}
}

/****************************************************************************/
uint32_t TimerTimeUntilNextEvent(void)
{
	if ((!TimerLibInitalized) || (TimerList == NULL))
	{
		return 0;
	}
	else
	{
		return (((TimerT *) (TimerList->liData))->Expire);
	}
}

#ifdef TIMER_DEBUG
/****************************************************************************/
uint8_t TimerMaxNumTimersUsed(void)
{
	return maxNumTimers;
}
#endif /*TIMER_DEBUG*/

