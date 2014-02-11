#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "stm32f4_discovery.h"

#include "DispatcherTask.h"
#include "PixelTask.h"
#include "LEDTask.h"
#include "LCDTask.h"
#include "USBTask.h"
#include "MMCTask.h"

void DiscoveryInit ( )
{
	STM_EVAL_LEDInit ( 0 );
	STM_EVAL_LEDInit ( 1 );
	STM_EVAL_LEDInit ( 2 );
	STM_EVAL_LEDInit ( 3 );
};

xTimerHandle xTimer;
void disk_timerproc(void);
void vTimerCallback( xTimerHandle pxTimer )
{
	disk_timerproc ( );
};

int main(void)
{
	SystemInit ( );

	DiscoveryInit ( );

	long x = 1;

	// Create then start some timers. Starting the timers before the scheduler
	// has been started means the timers will start running immediately that
	// the scheduler starts.
	xTimer = xTimerCreate (
			( const signed char * ) "Timer", 	// Just a text name, not used by the kernel.
			10 / portTICK_RATE_MS, 				// The timer period in ticks.
			pdTRUE, 							// The timers will auto-reload themselves when they expire.
			( void * ) x, 						// Assign each timer a unique id equal to its array index.
			vTimerCallback 						// Each timer calls the same callback when it expires.
	);
	if ( xTimer == NULL )
	{
		// The timer was not created.
	}
	else
	{
		// Start the timer. No block time is specified, and even if one was
		// it would be ignored because the scheduler has not yet been
		// started.
		if( xTimerStart( xTimer, 0 ) != pdPASS )
		{
			// The timer could not be set into the Active state.
		}
	};

	xTaskCreate( DispatcherTask, ( signed char * ) "Dispatcher", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	xTaskCreate( PixelTask, ( signed char * ) "Pixel", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	xTaskCreate( LEDTask, ( signed char * ) "StatusLED", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	xTaskCreate( LCDTask, ( signed char * ) "Display", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	xTaskCreate( USBTask, ( signed char * ) "USB", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
// 	xTaskCreate( MMCTask, ( signed char * ) "SD-Card", 512, NULL, 1, NULL );
	vTaskStartScheduler ( );

	while ( 1 )
	{
	};

	return ( 0 );
};
