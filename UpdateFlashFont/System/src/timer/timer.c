#include "timer.h"

__IO uint32_t SystemTick = 0;

uint32_t hwclock(void)
{
		return SystemTick;
}

