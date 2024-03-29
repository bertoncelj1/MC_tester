#ifndef test_mng_h
#define test_mng_h



#define ST_TEST_PROG  2	 //stevilo testnih programov
#define MAX_TEST_PROG 10 //najvecje stevilo programov vrednost mora biti > 0 in > ST_TEST_PROG
#define TEST_PROGRAM_FIRST 0 //index prvega testnega programa
enum izbireTestnihProgramov{
  TEST_PROG_FLASH = TEST_PROGRAM_FIRST,
  TEST_PROG_NORM,
};


#define TESTNE_OPR_FIRST 1 //index prve testne operacije
typedef enum { 
  PREVERI_PINE = TESTNE_OPR_FIRST, 
  PREVERI_FLASH, 
  PREVERI_LEDICE, 
  PREVERI_TIPKE, 
  PREVERI_BACKLIGHT,
  PREVERI_FLASH_OFF,
  ZAKLJUCI
}e_TestneOperacije;


#define MAX_OPERACIJ  20 //najvecje stevilo operacij, ki jih ima lahko ena izbira
typedef struct {
  char *ime;					//opis programa
  int treOperacija; 				//trenutna izbrana operacija
  e_TestneOperacije operacijeID[MAX_OPERACIJ]; //nastete vse operacije, ki jih ta test izvede 
}s_testniProgram;



//public functions
void test_mng_init();
void LCD_getTestniProgram();
e_TestneOperacije getFirstOperation();
e_TestneOperacije getNextOperation();
e_TestneOperacije getOperation();
const char *getCurrentOperationStr();
int dobiNapredekOdxtek(int x);
const char *getOpisProgramaStr();

#else
	#error test_mng_h  already included! 
#endif
