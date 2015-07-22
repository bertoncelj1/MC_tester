#ifndef tipke_test_h
#define tipke_test_h

typedef enum{ENTER, DESNO, LEVO, GOR, DOL}e_tipTipke;

int tipke_2(void);
void tipke_test_init();

#define TkGor 0x10
#define TkLev 0x02
#define TkDes 0x08
#define TkEnt 0x04
#define TkDol 0x01
#define TkRept (TkGor+TkDol+TkLev+TkDes+TkEnt)

#else
	#error tipke_test_h  already included! 
#endif
