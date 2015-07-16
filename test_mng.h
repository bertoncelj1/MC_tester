#ifndef test_mng_h
#define test_mng_h



#define ST_TEST_PROG  0	 //stevilo testnih programov
#define MAX_TEST_PROG 10 //najvecje stevilo programov vrednost mora biti > 0 in > ST_TEST_PROG
enum izbireTestnihProgramov{
  TEST_PROG_FLASH,
  TEST_PROG_NORM,
};

#define ST_OPERACIJ  6
/*
typedef enum{
	ZACETEK,         
	CAKA_START,     
	KONTROLA_KABELNA,
	NAPAKA,       
	KONTROLA_LCD,    
	SERVICE_MODE,    
}testneOperacije;
*/

enum testneOperacije {
A,S,D,F,G,H   
};


#define MAX_OPERACIJ  20 //najvecje stevilo operacij, ki jih ima lahko ena izbira
typedef struct {
  char *ime;						//opis operacije
  int treOperacija; 				//trenutna izbrana operacija
  int stOperacij;					//stevilo operacij < MAX_OPERACIJ
  enum testneOperacije operacijeID[MAX_OPERACIJ]; //nastete vse operacije, ki jih ta test izvede 
}s_testniProgram;



//public functions
void test_mng_init();
void LCD_getTestniProgram();
int getFirstOperation();
int getNextOperation();
int getOperation();

#else
	#error test_mng_h  already included! 
#endif