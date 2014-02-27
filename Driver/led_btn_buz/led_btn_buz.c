#include "led_btn_buz.h"
#include "stm32f4_discovery.h"

unsigned ledTimerPeriods[LED_NUM_TIMER] = {
		LED_TIMER_PERIOD_FLASH,
		LED_TIMER_PERIOD_FAST,
		LED_TIMER_PERIOD_SLOW,
};

enum LED_STATE led_state[LED_NUM_LEDS];

/* the buzzer acts similar to a led */
enum LED_STATE buzzer_state;

void LedInit ( )
{
	STM_EVAL_LEDInit ( 0 );
	STM_EVAL_LEDInit ( 1 );
	STM_EVAL_LEDInit ( 2 );
	STM_EVAL_LEDInit ( 3 );
	STM_EVAL_PBInit ( BUTTON_USER, BUTTON_MODE_GPIO );
	STM_EVAL_PBInit ( BUTTON_RIGHT, BUTTON_MODE_GPIO );
	STM_EVAL_PBInit ( BUTTON_ENCODER, BUTTON_MODE_GPIO );
};

void LedTimerCallback ( long lTimerID )
{
	static uint8_t PulseCounter = 0;
	uint8_t i;

	for ( i = 0; i < LED_NUM_LEDS; i++ ) if ( led_state[i] == LED_STATE_OFF ) STM_EVAL_LEDOff ( i );
	for ( i = 0; i < LED_NUM_LEDS; i++ ) if ( led_state[i] == LED_STATE_ON ) STM_EVAL_LEDOn ( i );
	if ( buzzer_state == LED_STATE_OFF ) ;
	else if ( buzzer_state == LED_STATE_ON ) ;

	switch ( lTimerID )
	{
	case 0:
		if ( ( PulseCounter % 8 ) == 0 )
		{
			for ( i = 0; i < LED_NUM_LEDS; i++ )
			{
				if ( led_state[i] == LED_STATE_FLASH ) STM_EVAL_LEDOn ( i );
			}
			if ( buzzer_state == LED_STATE_FLASH ) ;
		}
		else if ( ( PulseCounter % 8 ) == 1 )
		{
			for ( i = 0; i < LED_NUM_LEDS; i++ )
			{
				if ( led_state[i] == LED_STATE_FLASH ) STM_EVAL_LEDOff ( i );
			}
			if ( buzzer_state == LED_STATE_FLASH ) ;
		}
		PulseCounter++;
		break;
	case 1:
		for ( i = 0; i < LED_NUM_LEDS; i++ ) if ( led_state[i] == LED_STATE_BLINK_FAST ) STM_EVAL_LEDToggle ( i );
		if ( buzzer_state == LED_STATE_BLINK_FAST ) ;
		break;
	case 2:
		for ( i = 0; i < LED_NUM_LEDS; i++ ) if ( led_state[i] == LED_STATE_BLINK_SLOW ) STM_EVAL_LEDToggle ( i );
		if ( buzzer_state == LED_STATE_BLINK_SLOW ) ;
		break;
	};
};
