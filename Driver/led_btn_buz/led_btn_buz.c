#include "led_btn_buz.h"
#include "stm32f4_discovery.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"


unsigned ledTimerPeriods[LED_NUM_TIMER] = {
		LED_TIMER_PERIOD_FLASH,
		LED_TIMER_PERIOD_FAST,
		LED_TIMER_PERIOD_SLOW,
};

/* the buzzer acts similar to a led, so there are one more states than leds */
enum LED_MODE led_mode[LED_NUM_LEDS+1];
uint8_t led_state[LED_NUM_LEDS+1]; // is the led on or off
uint8_t led_pulses[LED_NUM_LEDS+1]; // how many pulses to show

void LedTimerInit ( )
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	// Clock enable
	RCC_APB1PeriphClockCmd ( RCC_APB1Periph_TIM12, ENABLE );

	// Timer disable
	TIM_Cmd ( TIM12, DISABLE );

	// Timer init
	TIM_TimeBaseStructure.TIM_Period =  256 - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = 328 - 1; // 328 * 256 * 2 ~ 168.000 -> pwm = about 1KHz
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM12, &TIM_TimeBaseStructure);

	// Timer preload enable
	TIM_ARRPreloadConfig(TIM12, ENABLE);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 127;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM12, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM12, TIM_OCPreload_Enable);
};

void LedGPIOInit ( )
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	// Config des Pins als Digital-Ausgang
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// Alternative-Funktion mit dem IO-Pin verbinden
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_TIM12);

};

void LedOn ( uint8_t ledNum )
{
	if ( ledNum < LED_NUM_LEDS )
	{
		STM_EVAL_LEDOn ( ledNum );
	}
	else if ( ledNum == LED_NUM_LEDS ) // that's the buzzer
	{
		TIM_Cmd ( TIM12, ENABLE );
	};
};

void LedOff ( uint8_t ledNum )
{
	if ( ledNum < LED_NUM_LEDS )
	{
		STM_EVAL_LEDOff ( ledNum );
	}
	else if ( ledNum == LED_NUM_LEDS ) // that's the buzzer
	{
		TIM_Cmd ( TIM12, DISABLE );
	};
};

void LedToggle ( uint8_t ledNum )
{
	if ( ledNum < LED_NUM_LEDS + 1 )
	{
		if ( led_state[ledNum] == 0 )
		{
			if ( ledNum == LED_NUM_LEDS ) // that's the buzzer
				TIM_Cmd ( TIM12, ENABLE );
			else
				STM_EVAL_LEDOn ( ledNum );
			led_state[ledNum] = 1;
		}
		else
		{
			if ( ledNum == LED_NUM_LEDS ) // that's the buzzer
				TIM_Cmd ( TIM12, DISABLE );
			else
				STM_EVAL_LEDOff ( ledNum );
			led_state[ledNum] = 0;
		};
	};
};

void LedInit ( )
{
	LedGPIOInit ( );
	LedTimerInit ( );

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

	switch ( lTimerID )
	{
	case 0:
		// If requested state is OFF just switch off the led. This is checked in the short timer callback
		for ( i = 0; i < LED_NUM_LEDS + 1; i++ ) if ( led_mode[i] == LED_MODE_OFF ) LedOff ( i );

		// If requested state is ON just switch on the led. This is checked in the short timer callback
		for ( i = 0; i < LED_NUM_LEDS + 1; i++ ) if ( led_mode[i] == LED_MODE_ON ) LedOff ( i );

		if ( ( PulseCounter % LED_FLASH_PERIOD ) == 0 )
		{
			// switch led on on pulse 0
			for ( i = 0; i < LED_NUM_LEDS + 1; i++ )
			{
				if ( led_mode[i] == LED_MODE_FLASH )
				{
					if ( led_pulses[i] == 0 ) led_mode[i] = LED_MODE_OFF;
					else if ( led_pulses[i] != -1 ) LedOn ( i );
				};
			};
		}
		else if ( ( PulseCounter % LED_FLASH_PERIOD ) == LED_FLASH_DUTY )
		{
			// switch led off after DUTY has elapsed
			for ( i = 0; i < LED_NUM_LEDS + 1; i++ )
			{
				if ( led_mode[i] == LED_MODE_FLASH ) STM_EVAL_LEDOff ( i );
			}
		}
		PulseCounter++;
		break;
	case 1:
		for ( i = 0; i < LED_NUM_LEDS + 1; i++ ) if ( led_mode[i] == LED_MODE_BLINK_FAST )
		{
			if ( led_pulses[i] == 0 ) led_mode[i] = LED_MODE_OFF;
			else if ( led_pulses[i] != -1 ) LedToggle ( i );
		}
		break;
	case 2:
		for ( i = 0; i < LED_NUM_LEDS + 1; i++ ) if ( led_mode[i] == LED_MODE_BLINK_SLOW )
		{
			if ( led_pulses[i] == 0 ) led_mode[i] = LED_MODE_OFF;
			else if ( led_pulses[i] != -1 ) LedToggle ( i );
		}
		break;
	};
};

void LedModeSet ( uint8_t ledNum, enum LED_MODE ledMode, uint8_t ledPulses )
{
	if ( ledNum >= LED_NUM_LEDS + 1 ) return;
	led_mode[ledNum] = ledMode;
	led_pulses[ledNum] = ledPulses;
};
