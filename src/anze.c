#include "head.h"
#include "scheduler.h"
#include "graphics.h"
#include "timerb.h"
#include "testi/flash_test.h"
#include "test_mng.h"
#include "backlight_test.h"
#include "error_mng.h"
#include "pini_test.h"
#include "str_funkcije.h"
#include "tipke_test.h"

//osnnovna stalna stanja preverjanja naprave
//vsa ostala stanja preverjanaja spadajo pod OPERACIJE
typedef enum{
  ZACETEK, CAKA_START, CAKAJ_RESET, NAPAKA, OPERACIJE, PREVERJAJ, GET_TESTNI_PROGRAM
}e_KontrolaStanja;

e_KontrolaStanja kontrolaStanja = GET_TESTNI_PROGRAM;
e_TestneOperacije operacijeState;
unsigned char zakasnitev_ser_mode;
unsigned char zakasnitev_testa;

//privatne funkcije
void izvediOperacije();
int preveriLedice();
int preveriPine();
int operacijaFlash();
int preveri_backlight();
char kontrola_vstavljen_LCD(void);
int prev_kable_2(void);
int preveriTipke();
void izpisiKonec();
void drawLoadingBar();
void izpisiSporocilo(const char *sporocilo, const char *podSporocilo, int yZamik);



void potek_kontrole(void){

    
    /*
  TODO: add this to the program
    if(KGet(TkLev | TkDes)){
       kontrola = SERVICE_MODE;
       zakasnitev_ser_mode = 0; 
       zakasnitev_testa = 0;
       clear();
       OutDev = STDOUT_LCD_NORMAL_FONT; 
       GrX = 20;  GrY = 5;
       printf("%s","***LCD*****");
        
        GrX = 20;  GrY = 25; 
        printf("%s","***TEST****");
        
        GrX = 20;  GrY = 45; 
        printf("%s","***MODE****");
        
        LCD_sendC();
        apps_disable(LCD_TEST_APP);
    }
    */
    
        
 switch (kontrolaStanja){
    
   
   case ZACETEK:
        //pridobi prvo testno operacijo, ki jo bo izvajal
        operacijeState = getFirstOperation();
                
        led_diode(OFF);
        
        //izpise zacetno besedilo
        izpisiSporocilo("VSTAVI" "\n" "LCD" "\n" "DISPLAY", getOpisProgramaStr(), 8);
        
        kontrolaStanja = CAKA_START;
        
        LCD_input_port_2();
        
    case CAKA_START: 
        if (kontrola_vstavljen_LCD()){
            LCD_init_2();  //inicializira na nov priklopljen zaslon
            kontrolaStanja = PREVERJAJ;
            clear();
        }
        
        if(KGet(TkEnt)){
            kontrolaStanja = GET_TESTNI_PROGRAM;
        }
        break; 
    
 
    case GET_TESTNI_PROGRAM:
      LCD_getTestniProgram();
      kontrolaStanja = ZACETEK;
    break;
    
    
    //koncal je s preverjanjem in caka na reset 
    case CAKAJ_RESET:
      led_diode(GREEN);
      // caka da stakne display
      if (kontrola_vstavljen_LCD()==0){
          kontrolaStanja = ZACETEK;
      }
      
      //TODO izpisi kaj se zgodi ko se pritisn tipka
      if(KGet(TkEnt)){
          kontrolaStanja = ZACETEK;
      }
      break;
    
    //ce v katerih izmed operacijpride do napake poklicejo to stanje
    //operacije je dolzna pred klicanjem izpisati na ekran napako 
    case NAPAKA: 
      led_diode(RED);
     // caka da stakne display
     if (kontrola_vstavljen_LCD()==0){
          kontrolaStanja = ZACETEK;
     }
      
     if(KGet(TkEnt)){
          kontrolaStanja = ZACETEK;
     }
     
    break; 
   
    //po vrsti se izvajajo operacije izbranega programa
    case PREVERJAJ:
    	izvediOperacije();
        
        if(KGet(TkEnt)){
          kontrolaStanja = ZACETEK;
        }
   	break;
 }
 
}

//izvaja operacije ki so dolocene za izbrani testniProgram definiran v test_mng.c
void izvediOperacije(){
    //na ekran izrise loading bar in izpise trenutno operacijo
    drawLoadingBar();  

    //ce operacije vrne 1 se bo program premaknil naprej na naslednjo operacijo
    int naprej = 0;
            
	switch (operacijeState){
	
		case KONTROLA_KABELNA:
		    naprej = preveriPine();
		    break;
		    
		case PREVERI_FLASH:
		    naprej = operacijaFlash();
		    break;
		    
		case PREVERI_LEDICE:
		    naprej = preveriLedice();
		    break;
		    
		case PREVERI_TIPKE:
                    //TODO popravi, da se bo napaka izvedla ze znotraj tipk
                    naprej = preveriTipke();
		    break;
		
		case PREVERI_BACKLIGHT:
		    naprej = preveri_backlight();
		    break;
                    
                case ZAKLJUCI:
                    izpisiSporocilo("TEST" "\n" "KONCAN", "odstranite napravo", 10);
                    kontrolaStanja = CAKAJ_RESET;
                    break;
	}
    
    //postavi nasljednjo operacijo    
    if(naprej){
      operacijeState = getNextOperation();
    }

}

void drawLoadingBar(){
    clearLine(0, 2);
    
    //izrise progress bar
    int napredek = dobiNapredekOdxtek(128);
    int vel = 2;
    char bar = (1 << vel) - 1;
    int i;
    for(i=128; i >= 128 - napredek; i--){
      LCD[i] = bar;
    }
    
    //izpise trenutno operacijo, ki jo testiramo
    OutDev = STDOUT_LCD;
    GrX =0;  GrY = 3;
    printf("test %s (%d%%)", getCurrentOperationStr(), dobiNapredekOdxtek(100));
    LCD_sendC();
}


int preveriLedice(){
  //poklice se app za testiranje ledic
  //opercije je vedno uspesa saj uporabnik sam preveri ledice
  timer_wait(lediceToggle_ID, 50);
  return 1;
}

int preveriPine(){

  //TODO: ugotovi zakaj je to tukaj?
  __delay_cycles(500000);

  //preveri ali je kratek stik na pinih
  if(prev_pine()){
    return 1;
  }
  
  //nasel je napako na enem pinu
  //izpise opozorilo
  //v primeru napake izpise obvestilo
  izpisiError("NAPAKA PINI", "pini v stiku:");	

  //gre v stanje NAPAKA kjer caka na reset
  kontrolaStanja =  NAPAKA;
  return 0;

}

int operacijaFlash(){
  if(preveriFlash()){
    return 1;
  }
  
  
  //v primeru napake izpise obvestilo
  izpisiError("NAPAKA SPOMINA", "flash output:");	
  
  //gre v stanje NAPAKA kjer caka na reset
  kontrolaStanja =  NAPAKA;
  return 0;

}

int preveri_backlight(){
  if(test_back_light()){
    return 1;
  }
  
  //v primeru napake izpise obvestilo
  izpisiError("NAPAKA BACKLIGHT", "opis napake:");
  
  //gre v stanje NAPAKA kjer caka na reset
  kontrolaStanja =  NAPAKA;
  return 0;
}

int preveriTipke(){
  int ret;
  if((ret = tipke_2()) >= 0){
    return ret;
  }
  
  //v primeru napake izpise obvestilo
  izpisiError("NAPAKA TIPKE", "hkrati pritisnjene:");
  
  //gre v stanje NAPAKA kjer caka na reset
  kontrolaStanja =  NAPAKA;
  
  return 0;
}


#define ST_CRK_NA_VRSTO 	20
#define Y_SIZE 				14	//koliko pix so vrstice narazen
#define SIR_EKR 			128	//sirina ekrana
#define SIR_FONTA			8
//izpise sporocilo na sredino ekrana
//yZamik -> koiko bo sporocilo zamaknjeno na dol
void izpisiSporocilo(const char *sporocilo, const char *podSporocilo, int yZamik){
	//clearLine(2, 8);
        clear();
	
	int yLine = yZamik; //trenutna pozicija vrstice na y osi
	int i = 0;		//mesto kjer se bere sporcilo
        int len;                //dolzina vrstce
	
	OutDev = STDOUT_LCD_NORMAL_FONT; //nasltavi font
	while(*sporocilo){		
		//poisce konec vrstice
                len = 0;
		while(*sporocilo != '\n' && len < ST_CRK_NA_VRSTO && *sporocilo != 0){
			i ++;
                        sporocilo ++;
                        len ++;
		}
		
		//izpise vrstico
		GrX = SIR_EKR/2 - (len*SIR_FONTA)/2; GrY = yLine;
		printf("%.*s", len, &sporocilo[-len]);
		yLine += Y_SIZE;
                
                //ce je nasel znak \n mora povecati i zato da ga v nasljednji iritaciji izspusti
                if(*sporocilo == '\n'){
                  i++;
                  sporocilo ++;
                }
	}
        
        OutDev = STDOUT_LCD;
        GrX = SIR_EKR/2 - (strLen(podSporocilo)*6)/2; GrY = yLine;
        printf("%s", podSporocilo);
        
	
	LCD_sendC();
	
}




char kontrola_vstavljen_LCD(void){ 
    P2DIR |= 0x03;  P1DIR |= 0xCF; 
    P2OUT |= 0x03;  P1OUT |= 0xCF;  //postavi na 1 da tudi ce pritiskas tipko ne kvari detekcije
    
    P3DIR &= ~0x01; // tipke na vhod 
    P3REN &= ~0x01; //  
    
    //__delay_cycles(10000); //normalizacija nivojev
    
    if (P3IN & 0x01){
        //__delay_cycles(10000); //se enkrat preveri

        if (P3IN & 0x01){
            P3REN |= 0x01; //  

            return 1;
        }   
    }
    P3REN |= 0x01; 
    
    return 0;
}

