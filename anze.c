#include "head.h"
#include "scheduler.h"
#include "graphics.h"
#include "timerb.h"
#include "flash_test.h"
#include "test_mng.h"
#include "backlight_test.h"

//osnnovna stalna stanja preverjanja naprave
//vsa ostala stanja preverjanaja spadajo pod OPERACIJE
typedef enum{
  ZACETEK, CAKA_START, CAKAJ_RESET, NAPAKA, OPERACIJE, PREVERJAJ
}e_KontrolaStanja;

e_KontrolaStanja kontrolaStanja = ZACETEK;
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
void izpisiKonec();
void drawLoadingBar();



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
        
        //ugasne back light //TODO kako je s tem ?
        P4LATCH2 = 0x00;
        LE573set_2();
        LE573hold_2();
        
        led_diode(OFF);
        
        //izpise zacetno besedilo
        izpisiSporocilo("VSTAVI" "\n" "LCD" "\n" "DISPLAY");
        
        kontrolaStanja = CAKA_START;
        
        LCD_input_port_2();
        
    case CAKA_START: 
        if (kontrola_vstavljen_LCD()){
            LCD_init_2();  //inicializira na nov priklopljen zaslon
            kontrolaStanja = PREVERJAJ;
            clear();
        }   

        break; 
    
    //koncal je s preverjanjem in caka na reset
    //TODO: dodaj da se bo dal resetirat s tipko    
    case CAKAJ_RESET:
      // caka da stakne display
      if (kontrola_vstavljen_LCD()==0){
          kontrolaStanja = ZACETEK;
      }   
      break;
    
    //ce v katerih izmed operacijpride do napake poklicejo to stanje
    //operacije je dolzna pred klicanjem izpisati na ekran napako 
    case NAPAKA: 
     //TODO dodej da se bo dal ponovit poiskus s tipko
     // caka da stakne display
     if (kontrola_vstavljen_LCD()==0){
          kontrolaStanja = ZACETEK;
     }
    break; 
   
    //po vrsti se izvajajo operacije izbranega programa
    case PREVERJAJ:
    	izvediOperacije();
   	break;
 }
 
}

//izvaja operacije ki so dolocene za izbrani testniProgram definiran v test_mng.c
void izvediOperacije(){
    //na ekran izrise loading bar in izpise trenutno operacijo
    drawLoadingBar();  

    //ce operacije vrne 1 se bo program premaknil naprej na salednjo operacijo
    //v primeru napake operacije vrnejo 0; Zato da ima uporabnik potem moznost 
    //resetirati in ponovno pognati neuspelo operacijo
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
            naprej = tipke_2();
		    break;
		
		case PREVERI_BACKLIGHT:
		    naprej = preveri_backlight();
		    break;
                    
        case ZAKLJUCI:
            //TODO comment ?
			//ugasne backlight
			P4LATCH2 = 0x00;
			LE573set_2();
			LE573hold_2();
				
			izpisiSporocilo("TEST" "\n" "KONCAN");
		
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
    
    //TODO izrise loadingbar
    int napredek = dobiNapredekOdxtotek(128);
    
    //izpise trenutno operacijo, ki jo testiramo
    OutDev = STDOUT_LCD;
    GrX =0;  GrY = 8;
    printf("testiram %s", getCurrentOperationStr());
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


#define ST_CRKN_NA_VRSTO 	20
#define Y_SIZE 				20	//koliko pix so vrstice narazen
#define SIR_EKR 			128	//sirina ekrana
#define SIR_FONTA			6
//izpise sporocilo na sredino ekrana
void izpisiSporocilo(char *sporocilo){
	clearLine(2, 8);
	
	int yLine = 20; 	//trenutna pozicija vrstice na y osi
	char line[stCrkNaVrsto+1];
	int len;		//dolzina trenutne vrstice
	
	OutDev = STDOUT_LCD_NORMAL_FONT; //nasltavi font
	while(*sporocilo){		
		//poisce vrstico in jo zapise v line
		len = 0;
		while(*pSpor != '\n' || len >= ST_CRKN_NA_VRSTO || *sporocilo == 0){
			line[len] = *sporocilo;
			len ++;
		}
		line[len] = 0;
		
		//izpise vrstico
		GrX = SIR_EKR/2 - (len*SIR_FONTA)/2; GrY = yLine;
		printf("%s", line);
		yLine += Y_SIZE;
		
	}
	
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

