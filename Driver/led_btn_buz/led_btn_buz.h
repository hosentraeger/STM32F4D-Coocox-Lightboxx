#ifndef __LED_BTN_BUZ_H__
#define __LED_BTN_BUZ_H__

#include "stm32f4xx.h"

#include "led_btn_buz_config.h"

enum LED
{
	LED_GREEN,
	LED_YELLOW,
	LED_RED,
	LED_BLUE,
	LED_NUM_LEDS
};

enum BUTTON
{
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_ENCODER,
	LED_NUM_BUTTONS
};

enum LED_TIMER
{
	LED_TIMER_FLASH,
	LED_TIMER_FAST,
	LED_TIMER_SLOW,
	LED_NUM_TIMER
};

enum LED_MODE
{
	LED_MODE_OFF,			// led is always off
	LED_MODE_BLINK_FAST,	// led blinks fast (even on- and off-time)
	LED_MODE_BLINK_SLOW,	// led blinks slow (even on- and off-time)
	LED_MODE_FLASH,		// short flash, long pause
	LED_MODE_ON			// led is always on
};

enum LED_BUTTON_STATE
{
	LED_BUTTON_STATE_NOT_PRESSED,
	LED_BUTTON_STATE_SHORT_PRESSED,
	LED_BUTTON_STATE_LONG_PRESSED,
};

void LedInit ( );
void LedTimerCallback ( long ulTimerID );
void LedModeSet ( uint8_t ledNum, enum LED_MODE ledMode, uint8_t ledPulses );
enum LED_BUTTON_STATE LedButtonGet ( enum BUTTON button );
extern unsigned ledTimerPeriods[LED_NUM_TIMER];

#endif //  __LED_BTN_BUZ_H__
