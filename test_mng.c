#include "test_mng.h"
#include  "graphics.h"

/*DEFININICIJE TESTNIH PROGRAMOV*/
//obvestilo: pri dodajanju novega testnega programa ne pozabite popraviti ST_TEST_PROG

static s_testniProgram testFlash = {
  "prog flash",			//opis programa
  0,					//trenutna testna operacija
  4,					//stevilo operacij
  KONTROLA_KABELNA, PREVERI_BACKLIGHT, PREVERI_TIPKE //nastete vse operacije, ki jih ta test izvede      
};

static s_testniProgram testNormalen = {
  "prog normalen",		//opis programa
  0,					//trenutna testna operacija
  4,					//stevilo operacij
  KONTROLA_KABELNA, PREVERI_BACKLIGHT, PREVERI_LEDICE, PREVERI_FLASH, PREVERI_TIPKE //nastete vse operacije, ki jih ta test izvede      
};


static s_testniProgram *testniProgrami[MAX_TEST_PROG];
static int izbranTesniProgram = -1;

//privatne funkcije
void izpisiIzbiro(int izbira, int leviZnak, int desniZnak);


void test_mng_init(){
	testniProgrami[TEST_PROG_FLASH] = &testFlash;
	testniProgrami[TEST_PROG_NORM] = &testNormalen;
}

//vrne nasljednjo operacijo; ce ta ne obstaja vrne -1
e_TestneOperacije getNextOperation(){
	if(izbranTesniProgram < 0)while(1); //NAPAKA, trenutna izbira ni dolocena
	if(testniProgrami[izbranTesniProgram]->treOperacija++ > testniProgrami[izbranTesniProgram]->stOperacij){
		return -1;
	}
	
	return testniProgrami[izbranTesniProgram]->treOperacija;
}

//vrne prvo operacijo testnega program
e_TestneOperacije getFirstOperation(){
	if(izbranTesniProgram < 0)while(1); //NAPAKA, trenutna izbira ni dolocena
	return testniProgrami[izbranTesniProgram]->treOperacija = 0;
}

e_TestneOperacije getOperation(){
  return testniProgrami[izbranTesniProgram]->treOperacija;
}





//na zaslon izrise vmesnik s katermi lahko uporabnik izbere testni program
void LCD_getTestniProgram(){
  clear();
  OutDev = STDOUT_LCD; 
  int xPoz = 15;
  GrX = xPoz;  GrY = 5;
  printf("Izberite tip");
    
  GrX = xPoz;  GrY = 15; 
  printf("naprave, ki jo");
  
  GrX = xPoz;  GrY = 25; 
  printf("zelite testirati:");
  
  if(ST_TEST_PROG == 0){
	GrX = xPoz;  GrY = 45; 
	printf("ni programov :(");
	LCD_sendC();
	izbranTesniProgram = -1;
	while(1); //ERROR: ni definiranih testnih programov
  }
  
  
  LCD_sendC();
  
  int trenutnaIzbira = 0;
  int tipkaDol = 0;
  int desno, levo; //oznacuje ce se bo narisal levi/desni znak
  desno = (ST_TEST_PROG == 1)? 0 : 1;
  levo = 0;
  izpisiIzbiro(trenutnaIzbira, levo, desno);
  while(1){
    BeriKey();
  
    if(KGet(TkLev) && levo){
      if(tipkaDol == 0){
        trenutnaIzbira--;
        desno = 1;
        if(trenutnaIzbira <= 0){
          trenutnaIzbira = 0;
          levo = 0;
        }
        izpisiIzbiro(trenutnaIzbira, levo, desno);
      }
      tipkaDol = 1;
    }
    
    else if(KGet(TkDes) && desno){
      if(tipkaDol == 0){
        trenutnaIzbira++;
        levo = 1;
        if(trenutnaIzbira >= ST_TEST_PROG - 1){
          trenutnaIzbira = ST_TEST_PROG - 1;
          desno = 0;
        }
        izpisiIzbiro(trenutnaIzbira, levo, desno);
      }
      tipkaDol = 1;
    }
    
    else if(KGet(TkEnt)){
      izbranTesniProgram = trenutnaIzbira;
      testniProgrami[izbranTesniProgram]->treOperacija = 0;
      return;
    }
    
    else if (tipkaDol == 1){
      tipkaDol = 0;
    }
    
    

  }
}

int strLen(char *str){
  int len = 0;
  while(*str++)len ++;
  return len;
}

//izpise izbiro na ekran tako da uporabnik lahko vidi kva izbira
void izpisiIzbiro(int izbira, int leviZnak, int desniZnak){
  int sir = 128;
  int margin = 5;
  int sirNormFonta = 6;
  int sirLCDFonta = 6;
  int visinaIzpisa = 45;
  int sirBesedila = strLen(testniProgrami[izbira]->ime)*sirLCDFonta;
  
  //pobrise od 5. do 6. vrstice
  clearArea(5*128, 7*128);
  
  OutDev = STDOUT_LCD_NORMAL_FONT;
  
  if(leviZnak){
    GrX = margin;  
    GrY = visinaIzpisa; 
    printf("<");
  }
  
  if(desniZnak){
    GrX = sir - sirNormFonta - margin;  
    GrY = visinaIzpisa; 
    printf(">");
  }
  
  OutDev = STDOUT_LCD;
  GrX = sir/2 - sirBesedila/2;
  GrY = visinaIzpisa + 1; 
  printf(testniProgrami[izbira]->ime);
  
  LCD_sendC();
}





