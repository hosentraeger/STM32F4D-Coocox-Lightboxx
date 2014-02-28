#include "led_btn_buz.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"

const uint16_t ledGPIO_PIN[LED_NUM_LEDS] =
{
		GPIO_Pin_12,
		GPIO_Pin_13,
		GPIO_Pin_14,
		GPIO_Pin_15,
};

unsigned ledTimerPeriods[LED_NUM_TIMER] =
{
		LED_TIMER_PERIOD_FLASH,
		LED_TIMER_PERIOD_FAST,
		LED_TIMER_PERIOD_SLOW,
};

const uint16_t ledBUTTON_PIN[LED_NUM_BUTTONS] =
{
		GPIO_Pin_2,
		GPIO_Pin_3,
		GPIO_Pin_10,
};

GPIO_TypeDef* ledBUTTON_PORT[LED_NUM_BUTTONS] =
{
		GPIOD,
		GPIOD,
		GPIOE,
};

/* the buzzer acts similar to a led, so there are one more states than leds */
enum LED_MODE led_mode[LED_NUM_LEDS+1];
enum LED_BUTTON_STATE button_state[LED_NUM_BUTTONS]; // was the button pressed / long-pressed
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
	TIM_TimeBaseStructure.TIM_Prescaler = 82 - 1; // 82 * 256 * 2 = 41984 -> pwm = about 4KHz
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

	// Buzzer
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// enable alternate function
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_TIM12);

	/* Configure the GPIO_LED pins */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* Enable the BUTTONs Clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	/* Configure Button pins as input */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	TIM12->CCER &= (uint16_t)~TIM_CCER_CC1E;
};

void LedOn ( uint8_t ledNum )
{
	if ( ledNum < LED_NUM_LEDS )
	{
		GPIOD->BSRRL = ledGPIO_PIN[ledNum];
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
		GPIOD->BSRRH = ledGPIO_PIN[ledNum];
	}
	else if ( ledNum == LED_NUM_LEDS ) // that's the buzzer
	{
		TIM_Cmd ( TIM12, DISABLE );
	};
};

void LedToggle ( uint8_t ledNum )
{
	if ( ledNum < LED_NUM_LEDS )
	{
		GPIOD->ODR ^= ledGPIO_PIN[ledNum];
	}
	else if ( ledNum == LED_NUM_LEDS ) // that's the buzzer
	{
		if ( 0 == ( TIM12->CR1 & TIM_CR1_CEN ) )
		{
			TIM_Cmd ( TIM12, ENABLE );
		}
		else
		{
			TIM_Cmd ( TIM12, DISABLE );
		};
	};
};

void LedInit ( )
{
	LedGPIOInit ( );
	LedTimerInit ( );
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
		for ( i = 0; i < LED_NUM_LEDS + 1; i++ ) if ( led_mode[i] == LED_MODE_ON ) LedOn ( i );

		if ( ( PulseCounter % LED_FLASH_PERIOD ) == 0 )
		{
			// switch led on on pulse 0
			for ( i = 0; i < LED_NUM_LEDS + 1; i++ )
			{
				if ( led_mode[i] == LED_MODE_FLASH )
				{
					if ( led_pulses[i] > 0 ) led_pulses[i]--;
					if ( led_pulses[i] == 0 ) led_mode[i] = LED_MODE_OFF;
					else LedOn ( i );
				};
			};
		}
		else if ( ( PulseCounter % LED_FLASH_PERIOD ) == LED_FLASH_DUTY )
		{
			// switch led off after DUTY has elapsed
			for ( i = 0; i < LED_NUM_LEDS + 1; i++ )
			{
				if ( led_mode[i] == LED_MODE_FLASH )
				{
					LedOff ( i );
					if ( led_pulses[i] > 0 ) led_pulses[i]--;
				}
			}
		}
		PulseCounter++;
		break;
	case 1:
		for ( i = 0; i < LED_NUM_LEDS + 1; i++ )
		{
			if ( led_mode[i] == LED_MODE_BLINK_FAST )
			{
				if ( led_pulses[i] == 0 )
				{
					led_mode[i] = LED_MODE_OFF;
					return;
				};
				LedToggle ( i );
				if ( led_pulses[i] < 0xff ) led_pulses[i]--;
			};
		};
		break;
	case 2:
		for ( i = 0; i < LED_NUM_LEDS + 1; i++ )
		{
			if ( led_mode[i] == LED_MODE_BLINK_SLOW )
			{
				if ( led_pulses[i] == 0 )
				{
					led_mode[i] = LED_MODE_OFF;
					return;
				};
				LedToggle ( i );
				if ( led_pulses[i] < 0xff ) led_pulses[i]--;
			};
		};
		break;
	};
};

void LedModeSet ( uint8_t ledNum, enum LED_MODE ledMode, uint8_t ledPulses )
{
	if ( ledNum >= LED_NUM_LEDS + 1 ) return;
	LedOff ( ledNum );
	led_mode[ledNum] = ledMode;
	led_pulses[ledNum] = ledPulses * 2;
};
