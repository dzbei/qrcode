/**
 * @file keys.h
 * @date 2009-July-14
 * @author S. Rohdemann, S.Radtke, A.Krause
 * @c (C) 2009 Nanotron Technologies
 * @brief Macros for key handling.
 *
 * @note BuildNumber = "pre-release"
 *
 * @note This file contains the macros for nanoLOC devkit board
 *       for accessing the keys.
 */
#ifndef	_KEYS_H
#define	_KEYS_H

#include "main.h"

#define NKEYS 		1

/*available key states*/

/*NOTE use the define KEY_PRESSED as mask
 *to find out if the key is currently pressed
 */
#define KEY_NOT_PRESSED		0x00		/*key is currently not pressed*/
#define KEY_REL_AFTER_10MS	0x0A		/*key released after 10 ms .. 1 sec */
#define KEY_REL_AFTER_1S	0x01		/*key released after 1..2 sec*/
#define KEY_REL_AFTER_2S	0x02		/*key released after 2..4 sec*/
#define KEY_REL_AFTER_3S	0x03		/*key released after 4..5 sec*/
#define KEY_REL_AFTER_4S	0x04		/*key released after 5..6 sec*/
#define KEY_REL_AFTER_5S	0x05		/*key released after 6 sec or more*/

#define KEY_PRESSED			0x80		/*key is currently pressed*/
#define KEY_PRESSED_FOR_1S 	0x81		/*key has been pressed for 1 second (still pressed yet)*/
#define KEY_PRESSED_FOR_2S 	0x82		/*key has been pressed for 2 second (still pressed yet)*/
#define KEY_PRESSED_FOR_3S 	0x83		/*key has been pressed for 3 second (still pressed yet)*/
#define KEY_PRESSED_FOR_4S 	0x84		/*key has been pressed for 4 second (still pressed yet)*/
#define KEY_PRESSED_FOR_5S 	0x85		/*key has been pressed for 5 second (still pressed yet)*/

#define KEY_PIN 		GPIO_Pin_4
#define KEY_PORT		GPIOE
/**
 * @brief Function pointer type for a function that is supposed to be called
 * when a key pressure starts.
 * @see KeyRegisterCallback
 *
 * @em kIndex Index of the key that was pressed.
 */
typedef void (*KeyPressCb_t)(uint8_t kIndex);


/**
 * @brief Function pointer type for a function that is supposed to be called
 * when a key was released.
 * @see KeyRegisterCallback
 *
 * @em kIndex Index of the key that was pressed.<b>
 * @em kPressDuration Total duration of the key pressure.
 */
typedef void (*KeyReleaseCb_t)(uint8_t kIndex, uint32_t kPressDuration);


/**
 * @brief Registers callback function for the events <i>key pressure</i> and
 * <i>key release</i>.
 * @param kPressCb Pointer to the callback function which will be called if
 * a key pressure starts. Set NULL to disable this feature.
 * @param kReleaseCb Pointer to the callback function which will be called if
 * a key pressure finished. Set NULL to disable this feature.
 *
 * Both callback function pointers are initialised to NULL as default,
 * i.e. nothing special happens by default on key pressure/release.
 */
extern void KeyRegisterCallback(KeyPressCb_t kPressCb, KeyReleaseCb_t kReleaseCb);


/**
 * @brief Gets the state of the key given by index.
 * @note <ul>
 * <li>This function is obsolete and should not be used for new projects. Use
 * the key press/release event mechanism provided by @ref KeyRegisterCallback ),
 * instead.</li>
 * <li>To find out if the key is currently pressed this sentence must be true:
 * (GetKeyState(mykey) & KEY_PRESSED) == KEY_PRESSED</li>
 * </ul>
 * @param keyIndex Index number of the key.
 * @returns One of the following values will be returned as long as the given
 * key is pressed/not pressed:
 * <table>
 * <tr><td><b>Value</b></td><td><b>Description</b></td></tr>
 * <tr><td>@ref KEY_NOT_PRESSED </td><td>if the key is currently not pressed,<td></tr>
 * <tr><td>@ref KEY_PRESSED </td><td>key has been pressed for < 1 sec,</td></tr>
 * <tr><td>@ref KEY_PRESSED_FOR_1S </td><td>if the key has been pressed for 1..2 sec,</td></tr>
 * <tr><td>@ref KEY_PRESSED_FOR_2S </td><td>if the key has been pressed for 2..3 sec,</td></tr>
 * <tr><td>@ref KEY_PRESSED_FOR_3S </td><td>if the key has been pressed for 3..4 sec,</td></tr>
 * <tr><td>@ref KEY_PRESSED_FOR_4S </td><td>if the key has been pressed for 4..5 sec,</td></tr>
 * <tr><td>@ref KEY_PRESSED_FOR_5S </td><td>if the key has been pressed for > 5 sec,</td></tr>
 * </table>
 * There is a special behaviour @em once after key release. The function will
 * then return one of these values:
 * <table>
 * <tr><td><b>Value</b></td><td><b>Description</b></td></tr>
 * <tr><td>@ref KEY_REL_AFTER_10MS </td><td>if the key was released after < 1 sec,<td></tr>
 * <tr><td>@ref KEY_REL_AFTER_1S </td><td>if the key was released after 1..2 sec,</td></tr>
 * <tr><td>@ref KEY_REL_AFTER_2S </td><td>if the key was released after 2..3 sec,</td></tr>
 * <tr><td>@ref KEY_REL_AFTER_3S </td><td>if the key was released after 3..4 sec,</td></tr>
 * <tr><td>@ref KEY_REL_AFTER_4S </td><td>if the key was released after 4..5 sec,</td></tr>
 * <tr><td>@ref KEY_REL_AFTER_5S </td><td>if the key was released after > 5 sec,</td></tr>
 * </table>
 */
extern uint8_t GetKeyState(uint8_t keyIndex);


/**
 * @brief Initialises the key driver.
 */
extern void KeyInit( uint8_t enableInterrupt );


/**
 * @brief Polls the available keys. Also performs the key debouncing.
 *
 * Poll this function in short cycles.
 */
extern void KeyPoll(void);

extern void KeyUpdate (uint8_t keyIndex);
/**
 * @brief Returns the current position of the sliding switch.
 * @note This function is only available for the CLOPS parent unit
 * hardware platform.
 */
extern uint8_t GetSwitchPosition(void);

void APLKeyReleased( uint8_t key, uint32_t kPressDuration );
void KEY_Init(void);

#endif	/* _KEYS_H */
