#ifndef flash_test_h
#define flash_test_h

#include  "msp430x24x.h"


//TYPE DEFINE
#ifndef UINT
#define UINT
typedef unsigned char uint8_t;
typedef char int8_t;

typedef unsigned int uint16_t;
typedef int int16_t;
#endif

//PINS

//clock P2.3 		flash symbol: C
#define set_clock_1   	P2OUT |= (1 << 3)                   
#define set_clock_0  	P2OUT &= ~(1 << 3)
#define dir_clock_1	P2DIR |= (1 << 3); set_clock_0 //pin direction 1-out, 0-in

//input P2.2, 		flash symbol: DQ0
#define set_input_1   	P2OUT |= (1 << 2)                    
#define set_input_0  	P2OUT &= ~(1 << 2) 
#define get_input   	(P2IN & (1 << 2)) 
#define dir_input_1	P2DIR |= (1 << 2); set_input_0 //pin direction 1-out, 0-in
#define dir_input_0	P2DIR &= ~(1 << 2)

//output P3.3, 		flash symbol: DQ1
#define set_output_1   	P3OUT |= (1 << 3)                    
#define set_output_0  	P3OUT &= ~(1 << 3)
#define get_output   	(P3IN & (1 << 3)) 
#define dir_output_1	P3DIR |= (1 << 3); set_output_0 //pin direction 1-out, 0-in
#define dir_output_0	P3DIR &= ~(1 << 3)

//chip select P6.6 	flash symbol: S#
#define set_chipSel_1   P6OUT |= (1 << 6)                    
#define set_chipSel_0  	P6OUT &= ~(1 << 6) 
#define dir_chipSel_1	P6DIR |= (1 << 6); set_chipSel_0 //pin direction 1-out, 0-in

//FLASH COMMANDS
#define CMD_READ_ID 	0x9F		//command read id 0x9E ali 0x9F



void flash_test_init();
uint8_t preveriFlash();
//nevidna od zunaj, ni ravno prav da se klice ta funkcija kot sama, raje klici preveriFlash in potem preberi napako z getErrorLine
//uint8_t readId(uint8_t *manID, uint16_t *devID, uint8_t *uniqueID, uint8_t *uniqueLen);
void sendByte(uint8_t byte);
uint8_t readByte();

#else
	#error flash_test_h  already included! 
#endif
