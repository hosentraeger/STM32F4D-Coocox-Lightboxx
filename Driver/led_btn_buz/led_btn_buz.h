#ifndef __LED_BTN_BUZ_H__
#define __LED_BTN_BUZ_H__

#include "stm32f4xx.h"

#define LED_NUM_TIMER 3
#define LED_TIMER_PERIOD_FLASH 100
#define LED_TIMER_PERIOD_FAST 250
#define LED_TIMER_PERIOD_SLOW 500

#define LED_NUM_LEDS LEDn

enum LED_STATE
{
	LED_STATE_OFF,
	LED_STATE_BLINK_SLOW,
	LED_STATE_BLINK_FAST,
	LED_STATE_FLASH,
	LED_STATE_ON
};

void LedInit ( );
void LedTimerCallback ( long ulTimerID );

extern unsigned ledTimerPeriods[LED_NUM_TIMER];



#endif //  __LED_BTN_BUZ_H__
