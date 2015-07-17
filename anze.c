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
        //pridobi prvo testno operacijo ki jo bo izvajal
        operacijeState = getFirstOperation();
        
        //ugasne back light
        P4LATCH2 = 0x00;
        LE573set_2();
        LE573hold_2();
        
        
        //izpise zacetno besedilo
        //TODO dej te sporocila v funkcijo
        clear();
        OutDev = STDOUT_LCD_NORMAL_FONT; 
        GrX =25;  GrY = 5;
        printf("%s","VSTAVI");
        
        GrX =35;  GrY = 25; 
        printf("%s","LCD");
        
        GrX = 5;  GrY = 45; 
        printf("%s","  DISPLAY   ");
        
        LCD_sendC();
        kontrolaStanja =  CAKA_START;
        
        LCD_input_port_2();
        
    case CAKA_START: 
        if (kontrola_vstavljen_LCD()){

            //LCD_init_2();  //inicializira na nov priklopljen zaslon
            kontrolaStanja = PREVERJAJ;
            //clear();
        }   
      //WORK IN PROGRESS HERE!!!  
    if(preveriFlash()){
      P6DIR |= (0x08 | 0x10);
      P6OUT |=  0x10; //prižgi zeleno
      P6OUT &= ~0x08; //ugasne rdeèo
    }else{
      P6DIR |= (0x08 | 0x10);
      P6OUT =  ~0x10; //ugasne zeleno
      P6OUT |= 0x08; //prižgi rdeèo
    } 

        break; 
        
    case CAKAJ_RESET:
      // caka da stakne display
      if (kontrola_vstavljen_LCD()==0){
          kontrolaStanja = ZACETEK;
      }   
      break;
    
    //ce v katerih izmed operacijpride do napake poklicejo to stanje
    //operacije je dolzna pred klicanjem izpisati napako
    case NAPAKA: 
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
        //na ekran izrise loading bar
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
                    izpisiKonec();
                    kontrolaStanja = CAKAJ_RESET;
                    break;
	}
        
        if(naprej){
          operacijeState = getNextOperation();
        }

}
void drawLoadingBar(){
          //TODO
        clearLine(0, 1);
        OutDev = STDOUT_LCD; 
        GrX =0;  GrY = 0;
        printf("testiram %s", getCurrentOperationStr());
        LCD_sendC();
}
void izpisiKonec(){
    //TODO comment ?
    //ugasne backlight
    P4LATCH2 = 0x00;
    LE573set_2();
    LE573hold_2();
    
    clear();
    OutDev = STDOUT_LCD_NORMAL_FONT; 
    GrX =2;  GrY = 12;
    printf("%s","TEST");
    GrX =2;  GrY = 35; 
    printf("%s","KONCAN");
    LCD_sendC();
    
    KeyBuf_2[0]=KeyBuf_2[1]=0; 
}

int preveriLedice(){
  //poklice se app za testiranje ledic
  timer_wait(lediceToggle_ID, 50);
  return 1;
}

int preveriPine(){

  //TODO: ugotovi zakaj je to tukaj?
  __delay_cycles(500000);

  //preveri ali je kratek stik na pinih
  int napacenPin = prev_kable_2();
  if(napacenPin == 0){
    return 1;
  }
  
  //nasel je napako na enem pinu

  //izpise opozorilo
  clear();
  OutDev = STDOUT_LCD_NORMAL_FONT; 
  GrX =30;  GrY = 5;
  printf("NAPAKA");

  GrX =30;  GrY = 25; 
  printf("NA PINU %d",napacenPin);

  GrX = 30;  GrY = 45; 
  printf("JE STIK");
  LCD_sendC();

  //gre v stanje NAPAKA kjer caka na reset
  kontrolaStanja =  NAPAKA;
  return 0;

}

int operacijaFlash(){
  if(preveriFlash()){
    return 1;
  }
  
  //v primeru napake izpise obvestilo
  clear();
  OutDev = STDOUT_LCD_NORMAL_FONT;
  
  GrX =10;  GrY = 5;
  printf("NAPAKA SPOMINA");
  
  OutDev = STDOUT_LCD;
  int lineY = 20;
  GrX =5;  GrY = lineY;
  printf("Memory errorBuff:");
  lineY += 10;
  
  //prebere errorLine vrstico po vrstico
  uint8_t line[50];
  uint8_t lineLen = 20; //koliko znakov bo izpisal na vrstico
  uint8_t offset = 0;
  //povprasa po vrstici, jo ipise in se pomakne v naslednjo
  while(getErrorLine(line, lineLen, offset)){
    offset += lineLen;
    GrX =5;  GrY = lineY;
    printf("%s", line);
    lineY += 8;
  }
  LCD_sendC();
  
  //gre v stanje NAPAKA kjer caka na reset
  kontrolaStanja =  NAPAKA;
  return 0;

}

int preveri_backlight(){
  if(test_back_light()){
    return 1;
  }
  
  //v primeru napake izpise obvestilo
  clear();
  OutDev = STDOUT_LCD_NORMAL_FONT;
  
  GrX =0;  GrY = 5;
  printf("NAPAKA BACKLIGHT");
  
  OutDev = STDOUT_LCD;
  int lineY = 20;
  GrX =5;  GrY = lineY;
  printf("Opis Problema:");
  lineY += 10;
  
  //prebere errorLine vrstico po vrstico
  uint8_t line[50];
  uint8_t lineLen = 20; //koliko znakov bo izpisal na vrstico
  uint8_t offset = 0;
  //povprasa po vrstici, jo ipise in se pomakne v naslednjo
  while(getErrorBacklightLine(line, lineLen, offset)){
    offset += lineLen;
    GrX =5;  GrY = lineY;
    printf("%s", line);
    lineY += 8;
  }
  LCD_sendC();
  
  //gre v stanje NAPAKA kjer caka na reset
  kontrolaStanja =  NAPAKA;
  return 0;
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


int prev_kable_2(void){
    // pin 3 tipka, 4 GND in 5 Vcc se izpustijo
    char testP1[6] = {0x01,0x02,0x04,0x08,0x40,0x80};
    char pinP1[6]  = { 11,  13,  15,  17,  18,  16 };
    char testP2[7] = {0x01,0x02,0x04,0x08,0x10,0x40,0x80};
    char pinP2[7]  = { 14,  12,  10,  8,   6,   2,   1 };
    char testP3[3] = {0x04,0x08};
    char pinP3[3]  = {  7 ,  9 };
    
    char P1,P2,P3;
    char pin;
    
    P1 = 0x01 | 0x02 |0x04 | 0x08 | 0x40 | 0x80;
    P2 = 0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x40 | 0x80;
    P3 = 0x01 | 0x04 | 0x08;
    //int sestevek = 0;
    //testira P1
    //return 1; 
    for(int i = 0; i < 6; ++i)
    {
        P1DIR = 0;  P1REN = P1;  P1OUT = 0;
        P2DIR = 0;  P2REN = P2;  P2OUT = 0;
        P3DIR = 0;  P3REN = P3;  P3OUT = 0;
        
        P1DIR  = testP1[i]; 
        P1OUT =  testP1[i];
        P1REN = ~ testP1[i];
        zakasni();
        pin = P1IN & ~(testP1[i] | 0x10 | 0x20);
        if (pin  != 0){
            
            return pinP1[i];
        }
        pin = P2IN & ~(0x20);
        if (pin  != 0){
            
            return pinP1[i];
        }
        pin = P3IN & ~(0x01 | 0x02 | 0x10 | 0x20 | 0x40 | 0x80);
        if (pin  != 0){// preskoèi se tipka
            
            return pinP1[i];
        }
    }
    
    
    //test P2
    for(int i = 0; i < 7; ++i)
    {
        P1DIR = 0;  P1REN = P1;  P1OUT = 0;
        P2DIR = 0;  P2REN = P2;  P2OUT = 0;
        P3DIR = 0;  P3REN = P3;  P3OUT = 0;
        
        P2DIR  = testP2[i]; 
        P2OUT =  testP2[i];
        P2REN = ~ testP2[i];
        zakasni();
        pin = P1IN & ~(0x10 | 0x20);
        if (pin  != 0){
            
            return pinP2[i];
        }
        pin = P2IN & ~(testP2[i] | 0x20);
        if (pin  != 0){
            
            return  pinP2[i];
        }
        pin = P3IN & ~(0x01 | 0x02 | 0x10 | 0x20 | 0x40 | 0x80);
        if (pin  != 0){// preskoèi se tipka
            
            return  pinP2[i];
        }
    }
    
    //test P3
    for(int i = 0; i < 2; ++i)
    {
        P1DIR = 0;  P1REN = P1;  P1OUT = 0;
        P2DIR = 0;  P2REN = P2;  P2OUT = 0;
        P3DIR = 0;  P3REN = P3;  P3OUT = 0;
        
        P3DIR  = testP3[i]; 
        P3OUT =  testP3[i];
        P3REN = ~ testP3[i];
        zakasni();
        pin = P1IN & ~(0x10 | 0x20);
        if (pin  != 0){
            
            return  pinP3[i];
        }
        
        pin = P2IN & ~(0x20);
        if (pin  != 0){
            
            return pinP3[i];
        }
        
        pin = P3IN & ~(testP3[i] | 0x01 | 0x02 | 0x10 | 0x20 | 0x40 | 0x80);
        if (pin  != 0){// preskoèi se tipka
            return pinP3[i];
        }
    }
    
    P1REN = 0;  P2REN = 0;  P3REN = 0;
    Init();
    LCD_init();
    LCD_init_2();
    return 0;
    
}
void zakasni(void){
    for(int i = 0; i < 200; ++i)
    {
        
    }
}
