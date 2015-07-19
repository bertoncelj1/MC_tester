#include "test_mng.h"
#include  "graphics.h"

/*PRIVATNE FUNKCIJE*/
void izpisiIzbiro(int izbira, int leviZnak, int desniZnak);
void addNumberToStr(char *str, int st, int mesto, int vel);
void prestejStOperacij();


/*DEFININICIJE TESTNIH PROGRAMOV*/
//obvestilo: pri dodajanju novega testnega programa ne pozabite popraviti ST_TEST_PROG
static s_testniProgram testFlash = {
  "prog flash",			//opis programa
  0,					//trenutna testna operacija
  KONTROLA_KABELNA, PREVERI_BACKLIGHT, PREVERI_LEDICE, PREVERI_FLASH, PREVERI_TIPKE, ZAKLJUCI //nastete vse operacije, ki jih ta test izvede      
  
};

static s_testniProgram testNormalen = {
  "prog normalen",		//opis programa
  0,				//trenutna testna operacija
  KONTROLA_KABELNA, PREVERI_BACKLIGHT, PREVERI_TIPKE, ZAKLJUCI //nastete vse operacije, ki jih ta test izvede      
};

/*PRIVATNE SPREMENLJIVKE*/
//uporablja za izpis imena testa, ki ga trenutno testiramo
static char *opisTesta[MAX_TEST_PROG];

static s_testniProgram *testniProgrami[MAX_TEST_PROG];
static int izbranTesniProgram = -1;
static int stOperacij; //stevilo operacij za trenutno izbrani program


void test_mng_init(){
        //init testne programe
	testniProgrami[TEST_PROG_FLASH] = &testFlash;
	testniProgrami[TEST_PROG_NORM] = &testNormalen;
        
        
        //init opis testov
        //opisi naj bodo dovolj kratiki da bodo primeri za izpis na ekranu
        opisTesta[KONTROLA_KABELNA] = "kabel";
        opisTesta[PREVERI_FLASH] = "flash";
        opisTesta[PREVERI_LEDICE] = "ledice";
        opisTesta[PREVERI_TIPKE] = "tipke";
        opisTesta[PREVERI_BACKLIGHT] = "backlight";
        opisTesta[ZAKLJUCI] = "konec";
}

//vrne nasljednjo operacijo; ce ta ne obstaja vrne -1
e_TestneOperacije getNextOperation(){
	if(izbranTesniProgram < 0)while(1); //NAPAKA, trenutna izbira ni dolocena
        
    int treOperacija = ++testniProgrami[izbranTesniProgram]->treOperacija;
        
        //ce je trenutna operacija 0 pomeni, da je prisel do konca, saj ni definirane nobene vec operacije
	if(testniProgrami[izbranTesniProgram]->operacijeID[treOperacija] == 0){
		return ZAKLJUCI;
	}
        
	return testniProgrami[izbranTesniProgram]->operacijeID[treOperacija];
}

//vrne prvo operacijo testnega program
e_TestneOperacije getFirstOperation(){
	if(izbranTesniProgram < 0)while(1); //NAPAKA, trenutna izbira ni dolocena
	testniProgrami[izbranTesniProgram]->treOperacija = 0;
	return testniProgrami[izbranTesniProgram]->operacijeID[0];
}

//vrne trenutno operacijo
e_TestneOperacije getCurrentOperation(){
    int treOperacija = testniProgrami[izbranTesniProgram]->treOperacija;
    return testniProgrami[izbranTesniProgram]->operacijeID[treOperacija];
}


const char *getCurrentOperationStr(){
  //uporablja v primeru ko je opisTesta prazen
  static char prazen[] = "null (XX)";
  static const int prazenMestoSt = 1;   //meso kamor bo zapisal st (prvi X)
  static const int prazenVelSt = 2; //velikost prostora za zapis stevila (st. X-sov)
  
  e_TestneOperacije operacijaID = getCurrentOperation();
  
  //pogleda ce je opisTesta definiran
  if(opisTesta[operacijaID] == NULL){
      //namesto XX zapise st. trenutne operacije operacije
      addNumberToStr(prazen, treOperacija, prazenMestoSt, prazenVelSt);
      return prazen;
  }
  
  return opisTesta[treOperacija];
}

//vrne kolikisen del x-sa je ze opravil
//odstotek pomeni del od sto, odxtotek pomeni del od x :)
int dobiNapredekOdxtotek(int x){
	return (testniProgrami[izbranTesniProgram]->treOperacija * x) / stOperacij;
}

//presteje stevilo operacij za trenutno izbran program. Rezultat zapise v stOperacij
void prestejStOperacij(){
  int treOperacija = testniProgrami[izbranTesniProgram]->treOperacija;
  stOperacij = 0;
  
  //v operacijah isce ZAKLJUCI ali pa 0
  while(testniProgrami[izbranTesniProgram]->operacijeID[treOperacija] != 0 &&
  		testniProgrami[izbranTesniProgram]->operacijeID[treOperacija] != ZAKLJUCI){
  	stOperacij++;		
  }
}


//na zaslon izrise vmesnik s katermi lahko uporabnik izbere testni program
void LCD_getTestniProgram(){
  clear();
  //izpise zgornji del besedila
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
  
  //pregleduje tipke in izpisuje trenutno izbrani program
  int trenutnaIzbira = TEST_PROGRAM_FIRST; //tnrenutno izbiro postavi na index prvega testnega programa
  int tipkaDol = 0;     //oznacuje, da uporabnik drzi tipko
  int desno, levo;      //oznacuje, ce se bo narisal levi/desni znak
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
      //postavi trenutno operacijo na zacetek
      testniProgrami[izbranTesniProgram]->treOperacija = 0;
      return;
    }
    //nobena tipka ni bila pritisnjena 
    else {
      tipkaDol = 0;
    }
  }
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





