#ifndef ledice_test_h
#define ledice_test_h

#include  "msp430x24x.h"


//TYPE DEFINE
typedef unsigned char uint8_t;
typedef char int8_t;

typedef unsigned int uint16_t;
typedef int int16_t;

//PINS
//notice: set latch on high, before changing led values!
//led alert for device 2, on pin P2.0
#define dir_ledAlert_1 	P2DIR |= 0x01       	//postavi smer registra na OUT in ugasne ledico
#define set_ledAlert_1  P2OUT |= 0x01;                 //vklopi ledico
#define set_ledAlert_0  P2OUT &= ~0x01;                  //izklopi ledico
#define get_ledAlert_out	(P2OUT & 0x01)

//notice: set latch on high, before changing led values!
//led pulse for device 2, on pin P1.7
#define dir_ledPulse_1  P1DIR |= 0x80       //postavi smer registra na OUT in ugasne ledico
#define set_ledPulse_1   P1OUT |= 0x80;                      //vklopi ledico
#define set_ledPulse_0  P1OUT &= ~0x80;                  //izklopi ledico
#define get_ledPulse_out	(P1OUT & 0x80)


//latch pin for 74HC573, on pin P6.7
#define dir_latch_1  P6DIR |= 0x80       
#define set_latch_1     P6OUT |= 0x80;                   //vklopi latch
#define set_latch_0   	P6OUT &= ~0x80;                 //izklopi latch
#define get_latch_out	(P6OUT & 0x80)         

void ledice_test_init();
//void toggle_ledAlert();
//void toggle_ledPulse();
void setLed();
void setLedNext();

#else
	#error ledice_test_h  already included! 
#endif