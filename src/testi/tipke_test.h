#ifndef tipke_test_h
#define tipke_test_h

#include "test_mng.h"

typedef enum{ENTER, DESNO, LEVO, GOR, DOL}e_tipTipke;

int tipke_2(void);
void* getTstOprTipke();

#else
	#error tipke_test_h  already included! 
#endif
