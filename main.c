#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "stm32f4_discovery.h"

#include "led_btn_buz.h"

#include "DispatcherTask.h"
#include "PixelTask.h"
#include "LCDTask.h"
#include "USBTask.h"
#include "MMCTask.h"
#include "AudioTask.h"

void disk_timerproc ( );

xTimerHandle xTimers[LED_NUM_TIMER+1];

void vTimerCallback ( xTimerHandle pxTimer )
{
	configASSERT( pxTimer );

	// Which timer expired?
	long lTimerID = ( long ) pvTimerGetTimerID ( pxTimer );

	if ( lTimerID < LED_NUM_TIMER ) LedTimerCallback ( lTimerID );
	else switch ( lTimerID )
	{
	case 4:
		disk_timerproc ( );
		return;
	};
};

xTimerHandle CreateStartTimer ( long id, unsigned period )
{
	xTimerHandle xTimer = xTimerCreate (
			( const signed char * ) "T",    	// Just a text name, not used by the kernel.
			period,				     			// The timer period in ticks.
			pdTRUE,         					// The timers will auto-reload themselves when they expire.
			( void * ) id,     					// Assign each timer a unique id equal to its array index.
			vTimerCallback     					// Each timer calls the same callback when it expires.
	);

	if( xTimer == NULL )
	{
		// The timer was not created.
	}
	else
	{
		// Start the timer.  No block time is specified, and even if one was
		// it would be ignored because the scheduler has not yet been
		// started.
		if( xTimerStart( xTimer, 0 ) != pdPASS )
		{
			// The timer could not be set into the Active state.
		}
	}
	return ( xTimer );
};

void SetupTimer ( )
{
	// Create then start some timers.  Starting the timers before the scheduler
	// has been started means the timers will start running immediately that
	// the scheduler starts.
	uint8_t x;

	// LED timer
	for ( x = 0; x < LED_NUM_TIMER; x++ )
	{
		xTimers[x] = CreateStartTimer ( x, ledTimerPeriods[x] / portTICK_RATE_MS );
	};

	// diskproc timer
	xTimers[x] = CreateStartTimer ( x, 10 / portTICK_RATE_MS );
};

int main(void)
{
	SystemInit ( );

	LedInit ( );

	SetupTimer ( );

	xTaskCreate( DispatcherTask, ( signed char * ) "Dispatcher", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	xTaskCreate( PixelTask, ( signed char * ) "Pixel", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	// xTaskCreate( LEDTask, ( signed char * ) "StatusLED", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	xTaskCreate( LCDTask, ( signed char * ) "Display", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	xTaskCreate( USBTask, ( signed char * ) "USB", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	xTaskCreate( MMCTask, ( signed char * ) "SD-Card", 512, NULL, 1, NULL );
	xTaskCreate( AudioTask, ( signed char * ) "USB", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	vTaskStartScheduler ( );

	while ( 1 )
	{
	};

	return ( 0 );
};
