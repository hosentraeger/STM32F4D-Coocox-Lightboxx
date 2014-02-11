#include "MMCTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4_discovery.h"
#include "stdio.h"
#include "ff.h"

/*
 * PC4 = CS
 * PC5 = CD (card detect)
 * PA5 = SCK
 * PA6 = MISO
 * PA7 = MOSI
 */

void MMCTask ( void * pvParameters )
{
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
		vTaskDelay ( 5000 / portTICK_RATE_MS );
	}
}
