#include "scheduler.h"
#include "graphics.h"
#include "graphics_LCD2.h"
#include "timerb.h"
#include "flash_test.h"
#include "test_mng.h"
#include "backlight_test.h"
#include "error_mng.h"
#include "pini_test.h"
#include "str_funkcije.h"
#include "tipke_test.h"
#include "main.h"
#include "run.h"
#include "ledice_test.h"

e_KontrolaStanja kontrolaStanja = GET_TESTNI_PROGRAM;
e_TestneOperacije operacijeState;

//uporablja za izpisovanja opravljenih operacij
int pozYOperationOK;


void potek_kontrole(void){
    
  switch (kontrolaStanja){
  case ZACETEK:
    //pridobi prvo testno operacijo, ki jo bo izvajal
    operacijeState = getFirstOperation();
    
    led_diode(OFF);
    
    //izpise zacetno besedilo
    izpisiSporocilo("VSTAVI" "\n" "LCD" "\n" "DISPLAY", getOpisProgramaStr(), 8);
    drawVersion(0,0);
    
    kontrolaStanja = CAKA_START;
    
    LCD_input_port_2();
    
  case CAKA_START: 
    if (kontrola_vstavljen_LCD()){
      LCD_init_2();  //inicializira na nov priklopljen zaslon
      kontrolaStanja = PREVERJAJ;
      pozYOperationOK = POZ_Y_OPR_OK_START; //postavi pozicijo kamor se bojo izpisovali ze preverjeni programi na zacetek
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
  case PREVERI_PINE:
    naprej = preveriPine();
    break;
    
  case PREVERI_FLASH:
    naprej = operacijaFlash();
    break;
    
  case PREVERI_LEDICE:
    naprej = preveriLedice();
    break;
    
  case PREVERI_TIPKE:
    naprej = preveriTipke();
    break;
    
  case PREVERI_BACKLIGHT:
    naprej = preveri_backlight();
    break;
    
  case PREVERI_FLASH_OFF:
    naprej = operacijaFlashOff();
    break;
    
  case ZAKLJUCI:
    izpisiSporocilo("TEST" "\n" "KONCAN", "odstranite napravo", 10);
    kontrolaStanja = CAKAJ_RESET;
    break;
  }
  
  //postavi nasljednjo operacijo    
  if(naprej){
    drawOperationOK();//preden nastavi novo stanje se izpise operacijo
    operacijeState = getNextOperation();
    initOperation();
  }
  
}

//inicializira operacijo preden jo izvede
void initOperation(){
  switch (operacijeState){
  case PREVERI_PINE:
    break;
  
  case PREVERI_FLASH_OFF: 
  case PREVERI_FLASH:
    flash_test_init();
    break;
    
  case PREVERI_LEDICE:
    ledice_test_init();
    break;
    
  case PREVERI_TIPKE:
    tipke_test_init();
    break;
    
  case PREVERI_BACKLIGHT:
    break;
    
  case ZAKLJUCI:
    break;
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


//na lokacijo (x, y) izpise verzijo programa
void drawVersion(int x, int y){
  GrX = x; GrY = y;
  printf("%s", getVersion());
  LCD_sendC();
}


//na levem delu zaslona izpise da se je operacija uspesno izvedla
void drawOperationOK(){
  OutDev = STDOUT_LCD; 
  GrX = 0; GrY = pozYOperationOK;
  printf("%s OK",  getCurrentOperationStr());
  LCD_sendC();
  pozYOperationOK += 8;
}


int preveriLedice(){
  //poklice se app za testiranje ledic
  //opercije je vedno uspesa saj uporabnik sam preveri ledice
  timer_wait(lediceToggle_ID, 50);
  return 1;
}

int preveriPine(){
  
  //TODO: ugotovi zakaj je to tukaj?
  //__delay_cycles(500000);
  
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

//isto kakor operacija flash, samo da je ta negirana
//uporablja se takrat, ko naprava ne sme vsebovati flasha
int operacijaFlashOff(){
  if(preveriFlashOff()){
    return 1;
  }
  
  //v primeru, da flash deluje izpise obvestillo
  izpisiError("NAPAKA SPOMINA", "flash je vklopljen!");	
  
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

