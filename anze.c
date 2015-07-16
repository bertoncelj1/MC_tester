#include "head.h"
#include "scheduler.h"
#include "graphics.h"
#include "timerb.h"
#include "flash_test.h"


enum KontrolaStanja{
  ZACETEK, CAKA_START, KONEC, NAPAKA, KONTROLA_KABELNA, 
  PREVERI_FLASH, PREVERI_LEDICE, PREVERI_TIPKE, PREVERI_BACKLIGHT,
};

enum KontrolaStanja kontrola;
unsigned char zakasnitev_ser_mode;
unsigned char zakasnitev_testa;




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
        LCD_init();
        LCD_init_2();
        off_REDled;
        off_GREENled;
        
        //ugasne back light
        P4LATCH = 0x00;
        LE573set();
        LE573hold();
        
        //ugasne back light
        P4LATCH2 = 0x00;
        LE573set_2();
        LE573hold_2();
        
        clear();
        OutDev = STDOUT_LCD_NORMAL_FONT; 
        GrX =25;  GrY = 5;
        printf("%s","VSTAVI");
        
        GrX =35;  GrY = 25; 
        printf("%s","LCD");
        
        GrX = 5;  GrY = 45; 
        printf("%s","  DISPLAY   ");
        
        LCD_sendC();
        kontrola =  CAKA_START;
        
        LCD_input_port_2();
        
        
    case CAKA_START: 
        if (kontrola_vstavljen_LCD()){
            kontrola = KONTROLA_KABELNA;
        }     

        break; 
        
    case KONEC:
      // èaka da stakne display
      if (kontrola_vstavljen_LCD()==0){
          kontrola = ZACETEK;
      }   
      break;
      
    case NAPAKA: 
      // èaka da stakne display
     if (kontrola_vstavljen_LCD()==0){
          kontrola = ZACETEK;
     }
    break; 
   
    
    /* PROCESI: */
    case KONTROLA_KABELNA: 
        preveriPine();
        break;
        
    case PREVERI_FLASH:
        preveriFlash();
        break;
        
    case PREVERI_LEDICE:
        preveriLedice();
        break;
        
    case PREVERI_TIPKE:
        break;
    
    case PREVERI_BACKLIGHT:
        
        break;
    //    return 0;
}

void preveriLedice(){
  //poklice se app za testiranje ledic
  timer_wait(lediceToggle_ID, 50);
}

void preveriPine(){

  //TODO: ugotovi zakaj je to tukaj?
  __delay_cycles(500000);

  //preveri ali je kratek stik na pinih
  napacenPin = prev_kable_2();
  if(napacenPin == 0){
    //TODO vse je k
  }else{
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
    kontrola =  NAPAKA;
  }  
}

void preveriFlash(){
  if(!preveriFlash()){
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
    kontrola =  NAPAKA;
  }

}

char kontrola_vstavljen_LCD(void){
    //return 1;   
    P2DIR |= 0x03;  P1DIR |= 0xCF; 
    P2OUT |= 0x03;  P1OUT |= 0xCF;  //postavi na 1 da tudi ce pritiskas tipko ne kvari detekcije
    
    P3DIR &= ~0x01; // tipke na vhod 
    P3REN &= ~0x01; //  
    __delay_cycles(10000); //normalizacija nivojev
    if (P3IN & 0x01){
        __delay_cycles(10000); //se enkrat preveri
        if (P3IN & 0x01){
            P3REN |= 0x01; //  
            return 1;
        }   
    }
    P3REN |= 0x01; //  
    return 0;
}




void back_light_error(int t){
      napaka = 1;
      LCD_vstavljen = 1;
      test_koncan = 1;

      clear();
      OutDev = STDOUT_LCD_NORMAL_FONT; 
      GrX =25;  GrY = 5;
      printf("NAPAKA !!!");
      GrX =15;  GrY = 25; 
      printf("BACK LIGHT");
      GrX = 25 ;  GrY = 45;
      
      if(t == 2){
        printf("TOK" );
      }
      else{
        printf("NAPETOST" );  
      }
      
      LCD_sendC();
      apps_disable(LCD_TEST_APP); 
      apps_enable(tipkeDVE_APP);
      timer_wait(tipkeDVE_ID,200);
}




void back_light_ON(void){

  P3DIR |= 0x02; // set port output 
  P3OUT |= 0x02; // set 1
}


void back_light_OFF(void){
  P3DIR |= 0x02; // set port output 
  P3OUT &= ~0x02; // set 0
}

char test_back_light(void){
  
    if( P3OUT & 0x02 ){ //preveri ali je Backlight vkljuèen potem lahko vkljuèim test
        
        P2DIR &= ~ 0x80;  // P2.7 ali je napetost 3-4V
        P2DIR &= ~ 0x40;  // P2.6 ali je tok veèji od 5mA
        if(!(P2IN & 0x80 )){
            return 3;
            //napetost ni dovolj velika
        }
        if(!(P2IN & 0x40 )){
            //tok ni dovolj velik
            return 2;
        }
    }
 
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
