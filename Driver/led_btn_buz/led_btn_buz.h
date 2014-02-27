#ifndef __LED_BTN_BUZ_H__
#define __LED_BTN_BUZ_H__

#include "stm32f4xx.h"

#define LED_NUM_TIMER 3
#define LED_TIMER_PERIOD_FLASH 100
#define LED_TIMER_PERIOD_FAST 250
#define LED_TIMER_PERIOD_SLOW 500

#define LED_NUM_LEDS LEDn
#define LED_FLASH_PERIOD 8 // this much LED_TIMER_PERIOD_FLASH milliseconds until next flash
#define LED_FLASH_DUTY 1 // this much LED_TIMER_PERIOD_FLASH milliseconds led on

enum LED_MODE
{
	LED_MODE_OFF,			// led is always off
	LED_MODE_BLINK_SLOW,	// led blinks slow (even on- and off-time)
	LED_MODE_BLINK_FAST,	// led blinks fast (even on- and off-time)
	LED_MODE_FLASH,		// short flash, long pause
	LED_MODE_ON			// led is always on
};

void LedInit ( );
void LedTimerCallback ( long ulTimerID );
void LedModeSet ( uint8_t ledNum, enum LED_MODE ledMode, uint8_t ledPulses );

extern unsigned ledTimerPeriods[LED_NUM_TIMER];

#endif //  __LED_BTN_BUZ_H__
