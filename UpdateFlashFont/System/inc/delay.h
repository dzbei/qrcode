#ifndef __DELAY_H
#define __DELAY_H

#include "stm32f10x.h"

void Delay(__IO uint32_t nCount);
void delay_nus(u16 time);
void delay_nms(u16 time);

#define Delay_us 	delay_nus
#define Delay_ms  delay_nms

#endif
