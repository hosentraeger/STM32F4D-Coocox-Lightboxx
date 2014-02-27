#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stm32f4_discovery.h"
#include "hd44780.h"
#include "LCDTask.h"

void LCDTask ( void * pvParameters )
{
	struct ALCDMessage *pxLCDMessage;

	xLCDQueue = xQueueCreate ( 8, sizeof ( unsigned long ) );

	hd44780_init (
			GPIOE,
			GPIO_Pin_7, GPIO_Pin_8, GPIO_Pin_9,					// RS, RW, E
			GPIO_Pin_10, GPIO_Pin_11, GPIO_Pin_12, GPIO_Pin_13, // DB5..DB8
			HD44780_LINES_2,
			HD44780_FONT_5x8 );

	while ( 1 )
	{
		if ( xQueueReceive ( xLCDQueue, & ( pxLCDMessage ), 10 / portTICK_RATE_MS ) )
		{
			hd44780_position ( pxLCDMessage->ucX, pxLCDMessage->ucY );
			hd44780_print ( pxLCDMessage->ucData );
			hd44780_display (true, false, false );
		}
	};
};
