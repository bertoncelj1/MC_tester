#ifndef backlight_test_h
#define backlight_test_h

#include "test_mng.h"

//TYPE DEFINE
#ifndef UINT
#define UINT
typedef unsigned char uint8_t;
typedef char int8_t;

typedef unsigned int uint16_t;
typedef int int16_t;
#endif


//javne funkcije
uint8_t getErrorBacklightLine(uint8_t *line, uint8_t lineLen, uint8_t errorOffset);
e_OprState preveri_backlight(void);
void* getTstOprBacklight();

#else
	#error backlight_test_h  already included! 
#endif
