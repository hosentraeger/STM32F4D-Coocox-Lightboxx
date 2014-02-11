*************************************************
SD CARD CONNECTION
*************************************************

 PC4 = CS					pink
 PC5 = CD (card detect)		<none>
 PA5 = SCK					orange
 PA6 = MISO					white
 PA7 = MOSI					gray

 SD Card
 ----------\	9
           |	1			CS
           |	2			MOSI
           |	3			GND
           | 	4			VDD
           |  	5			SCK
           |  	6			GND
           |  7/8			MISO
------------


 micro SD

     |-\
 --\ |  \--
    V     |		1			./.
          |		2			CS
          |		3			MOSI
          |		4			VDD
          |		5			SCK
          |		6			GND
          |		7			MISO
          |		8			./.
----------


*************************************************
LCD CONNECTION
*************************************************

HD44780

GND VDD CONTRAST RS    RW    E     DB0   DB1   DB2   DB3   DB4   DB5   DB6   DB7   BACKLIGHT-A BACKLIGHT-C

                 PE.07 PE.08 PE.09                         PE.10 PE.11 PE.12 PE.13
                 

*************************************************
LED STRIP CONNECTION
*************************************************

red 	-> +5V
white 	-> GND
green	-> PB.04 (Timer 3 Channel 1)



*************************************************
TSOP4838 Infrared Receiver
*************************************************
 -----
 | 0 |
 -----
 | | |
 | | |
 | | |
 1 2 3
 
 1 = DATA
 2 = GND
 3 = VDD
 
 
 