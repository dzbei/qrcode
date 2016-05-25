/**
 * @file keys.c
 * @date 2009-July-13
 * @c Copyright 2009 Nanotron Technologies
 * @brief This file contains the implementations for key functions.
 * @note BuildNumber = "pre-release"
 */
#include "keys.h"
#include "stdio.h"
#include "delay.h"
#include "app_task.h"
#include "timer.h"

/*constants for debouncing*/
#define	STATE_HIGH	 5
#define	STATE_LOW	-5

extern TaskStatus_TypeDef g_stTaskStatus;

/** @brief Internal data structure maintained for each single key.*/
typedef struct {
	int8_t		bounce;			/**< Debouncing counter.*/
	bool		pressed;		/**< TRUE if the key is recognised as pressed;
								 * FALSE otherwise.*/
	uint32_t	pressStartTime;	/**< Starting time of the last recent
								 * key pressure event.*/
} KeyStateT;


extern uint32_t hwclock(void);

static KeyPressCb_t keyPressCb = NULL;
static KeyReleaseCb_t keyReleaseCb = NULL;
static KeyStateT keyState[NKEYS] = {{STATE_LOW, FALSE, 0}};
static volatile uint8_t keyPressed[NKEYS] = {FALSE};
extern uint8_t  InterruptFlag;
extern void USART_printf(char *fmt, ...);

/****************************************************************************/
void KeyRegisterCallback(KeyPressCb_t kPressCb, KeyReleaseCb_t kReleaseCb)
{
	keyPressCb = kPressCb;
	keyReleaseCb = kReleaseCb;
}

void KEY_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

	//打开PE口时钟
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	//PE4引脚设置	
	GPIO_InitStructure.GPIO_Pin = KEY_PIN;
	//端口速度
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	//端口模式，此为输入上拉模式
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	//初始化对应的端口
  GPIO_Init(KEY_PORT, &GPIO_InitStructure);
}

/****************************************************************************/
/**
 * @brief Updates the state of a single key.
 * @param i Index of the key.
 *
 * This function updates the keystate for every hwclock tick. Its main purpose
 * is to eliminate key bouncing.
 */
void KeyUpdate (uint8_t keyIndex)
{
	KeyStateT *key = &(keyState[keyIndex]);
	uint8_t portPinState;

	switch (keyIndex)
	{
		case 0:
			/*get the current state of the key input pin.*/
			portPinState = ( uint8_t )GPIO_ReadInputData(KEY_PORT) & KEY_PIN;

			//按键按下时 portPinState 为1
			portPinState = ( uint8_t )(portPinState > 0 ? 0 : 1 );
			break;
		
			/* TODO Add any further keys here by index.*/

		default:
			/*unknown key index.*/
			return;
	}


	/*key debouncing*/
	if (portPinState != 0)
	{
		if(key->bounce > STATE_LOW)
		{
			(key->bounce)--;
		}
	}
	else
	{
		if(key->bounce < STATE_HIGH)
		{
			(key->bounce)++;
		}
	}
}

/****************************************************************************/
void KeyPoll(void)
{
	KeyStateT *key;
	uint8_t i;

	KeyUpdate (0);

	for (i = 0; i < NKEYS; i++)
	{
		key = &(keyState[i]);

		/*alternate key press state after debouncing finished*/
		if ((key->bounce == STATE_LOW) && !(key->pressed))
		{
			key->pressed = TRUE;
			key->pressStartTime = hwclock();
			//keyPressCb 为NULL
			if (keyPressCb != NULL)
			{
				/*invoke key press callback function*/
				(*keyPressCb)(i);
			}
		}
		else
		{
			if ((key->bounce == STATE_HIGH) && key->pressed)
			{
				key->pressed = FALSE;
			
				if (keyReleaseCb != NULL)
				{
					
					/*invoke key release callback function*/
					(*keyReleaseCb)(i, hwclock() - key->pressStartTime);
				}
			}
		}
	}
}

/****************************************************************************/
uint8_t GetKeyState(uint8_t keyIndex)
{
	KeyStateT *key = &(keyState[keyIndex]);
	uint32_t tDiff = hwclock() - key->pressStartTime;

	if (key->pressed)
	{
		/*has the key been just pressed?*/
		if (!keyPressed[keyIndex])
		{
			/*branch here once right after the key has been pressed*/
			keyPressed[keyIndex] = TRUE;
		}
		if (tDiff > 5000) return KEY_PRESSED_FOR_5S;
		if (tDiff > 4000) return KEY_PRESSED_FOR_4S;
		if (tDiff > 3000) return KEY_PRESSED_FOR_3S;
		if (tDiff > 2000) return KEY_PRESSED_FOR_2S;
		if (tDiff > 1000) return KEY_PRESSED_FOR_1S;
		return KEY_PRESSED;
	}
	else
	{
		/*has the key been just released?*/
		if (keyPressed[keyIndex])
		{
			/*branch here once right after the key has been released*/
			keyPressed[keyIndex] = FALSE;
			/*return key pressing duration*/
			if (tDiff > 5000) return KEY_REL_AFTER_5S;
			if (tDiff > 4000) return KEY_REL_AFTER_4S;
			if (tDiff > 3000) return KEY_REL_AFTER_3S;
			if (tDiff > 2000) return KEY_REL_AFTER_2S;
			if (tDiff > 1000) return KEY_REL_AFTER_1S;
			if (tDiff >   10) return KEY_REL_AFTER_10MS;
		}
		return KEY_NOT_PRESSED;
	}
}

/*******************************************************************/
void APLKeyReleased( uint8_t key, uint32_t kPressDuration )
/*******************************************************************/
{
		if(g_stTaskStatus == TASK_IDLE)
		{
				delay_nms(100);
				Task_HttpGetScanCodeList();
		}
}


