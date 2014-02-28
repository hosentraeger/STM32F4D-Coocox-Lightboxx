#include "FreeRTOS.h"
#include "task.h"
#include "ws2812.h"
#include "ColorWheelTable.h"
#include "PixelTask.h"

#define LED_COUNT 7

void PixelTask ( void * pvParameters )
{
	int16_t i;

	WS2812_init ( LED_COUNT );

	while ( 1 )
	{
		for (i = 0; i < 763; i += LED_COUNT)
		{
			WS2812_send(&eightbit[i], LED_COUNT);
			vTaskDelay ( 30 / portTICK_RATE_MS );
		};

		uint8_t colors[LED_COUNT][3];

		for ( i = 0; i < 128; i++ )
		{
			uint8_t j = 0;
			for ( j = 0; j < LED_COUNT; j++ )
			{
				colors[j][0] = 255 - i * 2;
				colors[j][1] = 0;
				colors[j][2] = 0;
			};
			WS2812_send(&colors[0], LED_COUNT);
			vTaskDelay ( 10 / portTICK_RATE_MS );
		};
		for ( i = 0; i < 128; i++ )
		{
			uint8_t j = 0;
			for ( j = 0; j < LED_COUNT; j++ )
			{
				colors[j][0] = i * 2;
				colors[j][1] = i * 2;
				colors[j][2] = i * 2;
			};
			WS2812_send(&colors[0], LED_COUNT);
			vTaskDelay ( 10 / portTICK_RATE_MS );
		};
		for ( i = 0; i < 128; i++ )
		{
			uint8_t j = 0;
			for ( j = 0; j < LED_COUNT; j++ )
			{
				colors[j][0] = 255;
				colors[j][1] = 255 - i * 2;
				colors[j][2] = 255 - i * 2;
			};
			WS2812_send(&colors[0], LED_COUNT );
			vTaskDelay ( 10 / portTICK_RATE_MS );
		};
	};
};
