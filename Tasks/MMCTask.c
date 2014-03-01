#include "MMCTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"
#include "ff.h"

#include "led_btn_buz.h"
/*
 * PC4 = CS
 * PC5 = CD (card detect)
 * PA5 = SCK
 * PA6 = MISO
 * PA7 = MOSI
 */

void MMCTask ( void * pvParameters )
{
	/*
	FATFS FatFs;
	FIL fil;
	FRESULT res;

	printf ( "Mounting volume...");
	res = f_mount ( &FatFs, "", 1 ); // mount the drive
	if ( res )
	{
		printf ( "error occured!\n" );
		while ( 1 );
	}
	printf ( "success!\n");
*/
	while ( 1 )
	{
/*
		char buffer[6];
		printf ( "Opening file: \"hello.txt\"...");
		res = f_open ( &fil, "hello.txt", FA_OPEN_EXISTING | FA_WRITE | FA_READ ); // open existing file in read and write mode
		if ( res )
		{
			printf ( "error occured!\n" );
			while ( 1 );
		}
		printf ( "success!\n" );

		f_gets ( buffer, 6, &fil );

		printf ( "I read: \"" );
		printf ( buffer );
		printf ( "\"\n" );

		f_puts ( buffer, &fil );

		f_close ( &fil ); // close the file
		f_mount ( NULL, "", 1 ); // unmount the drive
*/
		enum LED_BUTTON_STATE state;
		state = LedButtonGet ( BUTTON_LEFT );
		if ( LED_BUTTON_STATE_SHORT_PRESSED == state ) LedModeSet ( LED_GREEN, LED_MODE_BLINK_FAST, 5 );
		if ( LED_BUTTON_STATE_LONG_PRESSED == state ) LedModeSet ( LED_GREEN, LED_MODE_BLINK_SLOW, 5 );

		state = LedButtonGet ( BUTTON_RIGHT );
		if ( LED_BUTTON_STATE_SHORT_PRESSED == state ) LedModeSet ( LED_YELLOW, LED_MODE_BLINK_FAST, 5 );
		if ( LED_BUTTON_STATE_LONG_PRESSED == state ) LedModeSet ( LED_YELLOW, LED_MODE_BLINK_SLOW, 5 );

		state = LedButtonGet ( BUTTON_ENCODER );
		if ( LED_BUTTON_STATE_SHORT_PRESSED == state ) LedModeSet ( LED_BLUE, LED_MODE_BLINK_FAST, 5 );
		if ( LED_BUTTON_STATE_LONG_PRESSED == state ) LedModeSet ( LED_BLUE, LED_MODE_BLINK_SLOW, 5 );
		vTaskDelay ( 250 / portTICK_RATE_MS );
	};
};
