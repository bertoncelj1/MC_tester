#include "head.h"
#include "scheduler.h"
#include "graphics.h"
#include "timerb.h"
#include "flash_test.h"

#define ENTER 1
#define GOR 2
#define DOL 3
#define LEVO 4
#define DESNO 5
unsigned char vhod4;
unsigned char vhod5;
unsigned char spre_prvr;
unsigned char keri_del;
char blink, on;

int display_state;
void izrisi_tipko(int tip);
void  izpis_pritisnjenih_tipk(void);
char spre;
unsigned char kontrola;
unsigned char zakasnitev_ser_mode;
unsigned char zakasnitev_testa;
char napaka;
char LCD_vstavljen;
char test_koncan;


void sahovnica(void){
    int menjaj,i;
    int zamik;
    zamik = 1;
    menjaj = 1;
    i = 0;
    for(int b = 0; b < 8; ++b){ 
        menjaj = 0;
        
        if ( zamik == 1 ) {
            zamik = 0;
        }
        else{
            zamik = 1;
        }
        
        
        for(int j = 0; j < 128; ++j){ 
            if (zamik == 1){
                if (menjaj < 8){
                    LCD[i++]=0xFF;   // vsebina LCD prikazovalnika
                    menjaj++;
                }
                else if(menjaj < 16){
                    LCD[i++]=0x00;
                    menjaj++;
                }
                else{
                    LCD[i++]=0xFF;   // vsebina LCD prikazovalnika
                    menjaj=1;}
            }
            else
            {
                if (menjaj < 8){
                    LCD[i++]=0x00;   // vsebina LCD prikazovalnika
                    menjaj++;
                }
                else if(menjaj < 16){
                    LCD[i++]=0xFF;
                    menjaj++;
                }
                else{
                    LCD[i++]=0x00;   // vsebina LCD prikazovalnika
                    menjaj=1;
                }
            }
            
        }
     
    }
}
void sahovnica_inverzno(void){
    int menjaj,i;
    int zamik;
    zamik = 1;
    menjaj = 1;
    i = 0;
    for(int b = 0; b < 8; ++b){ 
        menjaj = 0;
        
        if ( zamik == 1 ) {
            zamik = 0;
        }
        else{
            zamik = 1;
        }
        
        /* for(int j = 0; j < 32; ++j){ 
        
        LCD[i++]=0x00;
        
    }*/
        
        
        for(int j = 0; j < 128; ++j){ 
            if (zamik == 1){
                if (menjaj < 8){
                    LCD[i++]=0x00;   // vsebina LCD prikazovalnika
                    menjaj++;
                }
                else if(menjaj < 16){
                    LCD[i++]=0xff;
                    menjaj++;
                }
                else{
                    LCD[i++]=0x00;   // vsebina LCD prikazovalnika
                    menjaj=1;}
            }
            else
            {
                if (menjaj < 8){
                    LCD[i++]=0xff;   // vsebina LCD prikazovalnika
                    menjaj++;
                }
                else if(menjaj < 16){
                    LCD[i++]=0x00;
                    menjaj++;
                }
                else{
                    LCD[i++]=0xff;   // vsebina LCD prikazovalnika
                    menjaj=1;
                }
            }
            //LCD[i-1]=j;
        }
        /*for(int j = 0; j < 32; ++j){ 
        
        LCD[i++]=0x00;
        
    }*/
        
        
    }
}
void pas(int i){
    for(int j = 512; j < 1024; ++j){ 
        
        LCD[i++]=0xFF;
        
    }
    
    
    
    
}
void zgoraj_belo(){
    for(int i = 512; i < 1024; ++i){ 
        LCD[i]=0xFF;
    }
}
void zgoraj_crno(){
    for(int i = 0; i < 512; ++i){ 
        LCD[i]=0xFF;
    }
    
}
void clear(){
    for(int i = 0; i < 1024; ++i){ 
        LCD[i]=0x00;
        
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

enum errorCodes{
  VSE_OK,
  NAPAKA_PINI,
  NAPAKA_FLASH,
};

void potek_kontrole(void){
    int t;
    int napacenPin;
    int errorCode;
    
    t=KGet();
    
    
    if(t==TkLev){
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
    
        
 switch (kontrola){
            
   case ZACETEK:
        LCD_vstavljen = 0;
        test_koncan = 0;
        napaka = 0;
        LCD_init();
        LCD_init_2();
        off_REDled;
        off_GREENled;
        flags=1;  // za risanje
        LCDpr=1;
        blink = 0;
        
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
        
        break;
        
    case CAKA_START: 
   
        if (kontrola_vstavljen_LCD()){
            kontrola = KONTROLA_KABELNA;
        }     

        break; 
        
    case KONTROLA_KABELNA: 
        LCD_vstavljen = 1;
        test_koncan = 0;
        napaka = 0;
        
        //poklice se app za testiranje ledic
        timer_wait(lediceToggle_ID, 50);
        
        __delay_cycles(500000);
        
        errorCode = VSE_OK;
        
        //preveri ali je kratek stik na pinih
        napacenPin = prev_kable_2();
        if(napacenPin){
          errorCode = NAPAKA_PINI;
        }
        
        //preveri ali deluje flash
        if(!preveriFlash()){
          errorCode = NAPAKA_FLASH;
        }
        
        switch(errorCode){
        case VSE_OK:
          clear();
          OutDev = STDOUT_LCD_NORMAL_FONT; 
          
          GrX =30;  GrY = 5; 
          printf("TEST LCD");
          LCD_sendC();
          
          OutDev = STDOUT_LCD; 
          GrX =15;  GrY = 22; 
          printf("- Spomin OK");
          
          GrX =15;  GrY = 32; 
          printf("- Konektor OK");
          
          
          GrX =50;  GrY = 55; 
          printf("1.03");
          LCD_sendC();
          OutDev = STDOUT_LCD_NORMAL_FONT; 
          
          //prizge back light
          P4LATCH2 = 0x01;
          LE573set_2();
          LE573hold_2();
          
          test_tipk_biti=0;
          display_state=0;
          //on_GREENled;
          display_test();
          off_REDled;
          off_GREENled;
          
          timer_wait(READ_KEY_ID, 50);
          timer_wait(LCD_TEST_ID,200);
          apps_enable(LCD_TEST_APP);
          
          kontrola =  KONTROLA_LCD;
          //apps_suspend(tipkeDVE_APP);
          break;
          
        case NAPAKA_PINI:
          clear();
          OutDev = STDOUT_LCD_NORMAL_FONT; 
          GrX =30;  GrY = 5;
          printf("NAPAKA");
          
          GrX =30;  GrY = 25; 
          printf("NA PINU %d",napacenPin);
          
          GrX = 30;  GrY = 45; 
          printf("JE STIK");
          LCD_sendC();
          kontrola =  NAPAKA;
          
          test_koncan = 1;
          napaka = 1;
          break;
          
        case NAPAKA_FLASH:
          clear();
          OutDev = STDOUT_LCD_NORMAL_FONT;
          GrX =10;  GrY = 5;
          printf("NAPAKA SPOMINA");
          
          
          OutDev = STDOUT_LCD;
          int lineY = 20;
          GrX =5;  GrY = lineY;
          printf("Memory errorBuff:");
          lineY += 10;
          
          uint8_t line[50];
          uint8_t lineLen = 20;
          uint8_t offset = 0;
          while(getErrorLine(line, lineLen, offset)){
            offset += lineLen;
            GrX =5;  GrY = lineY;
            printf("%s", line);
            lineY += 8;
          }
          LCD_sendC();
          
          
          kontrola =  NAPAKA;
          
          test_koncan = 1;
          napaka = 1;
          break;
        
        }
        break;
        
    case NAPAKA: 
        // èaka da stakne display
       if (kontrola_vstavljen_LCD()==0){
            kontrola = ZACETEK;
            LCD_vstavljen = 0;
            
       }   

        break;
        
        
    case KONTROLA_LCD:
        // èaka da stakne display
        if (kontrola_vstavljen_LCD()==0){
            LCD_vstavljen = 0;
            kontrola = ZACETEK;
        }   
   

        
        break;  
        
    case SERVICE_MODE:
       test_koncan = 0;
       clear();
       if(zakasnitev_ser_mode < 2){
          back_light_OFF(); 
          zgoraj_belo();
       }       
       else if (zakasnitev_ser_mode < 4)  {
          back_light_ON(); 
          zgoraj_crno();
       }
       else{
           back_light_OFF(); 
           zgoraj_belo();
           zakasnitev_ser_mode=0;
           zakasnitev_testa =1;
       }
       if((t==1)&& zakasnitev_testa){ 
            display_state = 0;
            kontrola =  ZACETEK;
       }
       zakasnitev_ser_mode++;
       LCD_sendC_2();   
       
       break;  
    }

   
    //    return 0;
}





int tipke_2(void){
    int t;
    int press;
    int tmp;
    
    //t=KGet_2();
    t=KeyBuf_2[0];
    KeyBuf_2[0] = 0;
    press=0;
    //(TkGor+TkDol+TkLev+TkDes+TkEnt)
    if(t){
        switch (t){
        case TkEnt: //enter
            if(test_tipk_biti & 0x01){
            }
            else{
                display_state++;
                //display_test();
                test_tipk_biti |= 0x01;
                press=1;
                
            }
            break;
            
        case TkDes: //Desno
            if(test_tipk_biti & 0x02){
            }
            else{
                test_tipk_biti |= 0x02;
                display_state++;
                //display_test();
                press=1;
            }
            
            break;
            
        case TkLev: //Levo
            if(test_tipk_biti & 0x04){
            }
            else{
                test_tipk_biti |= 0x04;
                display_state++;
                //display_test();
                press=1;
            }
            break;
            
        case TkGor: //Gor
            if(test_tipk_biti & 0x08){
            }
            else{
                test_tipk_biti |= 0x08;
                display_state++;
                //display_test();
                press=1;
            }
            break; 
        case TkDol: //Dol  
            if(test_tipk_biti & 0x10){
            }
            else{
                test_tipk_biti |= 0x10;
                display_state++;
                //display_test();
                press=1;
            }
            //cur_Y--;
            break;
        default:
            
            // napaka
            key_error(t);
            return 0;
            break;
            
        }
        display_test();
        izpis_pritisnjenih_tipk();
        tmp = test_back_light();
        if (tmp){
           //back light ne dela dobro
           back_light_error(tmp); 
           return 0; 
        }
        
        if(test_tipk_biti == 0x1F){
          return 1;
        }
        else{
          return 0;
        }
    }
    
    
    return 0;
}

void key_error(int t){
            napaka = 1;
            LCD_vstavljen = 1;
            test_koncan = 1;

            clear();
            OutDev = STDOUT_LCD_NORMAL_FONT; 
            GrX =20;  GrY = 5;
            printf("%s","NAPAKA !!!");
            GrX =5;  GrY = 25; 
            printf("%s","HKRATI PRITISN.");
            GrX = 3 ;  GrY = 45;
            printf("TIPKE:" );
            //(TkGor+TkDol+TkLev+TkDes+TkEnt)
            if ( t & TkEnt){
                printf(" OK," );
            }
            if ( t & TkGor){
                printf(" +," );
            }
            if ( t & TkDol){
                printf(" -," );
            }
            if ( t & TkLev){
                printf(" <," );
            }
            if ( t & TkDes){
                printf(" >," );
            }
            //ponovi test
            LCD_sendC();
            apps_disable(LCD_TEST_APP); 
            apps_enable(tipkeDVE_APP);
            timer_wait(tipkeDVE_ID,200);
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

void  izpis_pritisnjenih_tipk(void){
    clear();
    OutDev = STDOUT_LCD_NORMAL_FONT; 
    
    if(test_tipk_biti & 0x08){
        GrX =90;  GrY = 5;
        printf("%s","+");
    }
    if(test_tipk_biti & 0x10){
        GrX =90;  GrY = 50;
        printf("%s","-");
    }
    if(test_tipk_biti & 0x04){
        GrX =60;  GrY = 30;
        printf("%s","<");
    }
    if(test_tipk_biti & 0x02){
        GrX =120;  GrY = 30;
        printf("%s",">");
    }
    if(test_tipk_biti & 0x01){
        GrX =83;  GrY = 30;
        printf("%s","OK");
    }
    LCD_sendC();
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


void display_test(){
    clear();
    switch( display_state )
    {
    default:
        
    case 0:
        //zgoraj_belo();
        //display_state++;
        break;    
    case 1:
        back_light_ON();
        zgoraj_belo();
        
        //display_state++;
        break;
        
    case 2:
        back_light_OFF();
        zgoraj_crno();
        
        //display_state++;
        break;
    case 3:
        back_light_ON();
        sahovnica();
        
        //display_state++;
        break;
        
    case 4:
        back_light_OFF();
        sahovnica_inverzno();
        
        //display_state++;
        break;
        
    case 5:
        back_light_ON();
        zgoraj_crno();
        zgoraj_belo();
        
        break;
    }
    
    
    LCD_sendC_2();
    //LCD_sendC_2();
}


void izrisi_tipko(int tip){
    
    switch (tip){
    case ENTER:
        clear();
        plot(0,0);
        line(42,24);
        line(42,42);
        line(0,64);
        line(42,42);
        line(84,42);
        line(128,64);
        line(84,42);
        line(84,24);
        line(128,0);
        line(84,24);
        line(42,24);
        
        plot(62,29);
        line(65,29);
        line(67,31);
        line(67,34);
        line(65,36);
        line(62,36);
        line(60,34);
        line(60,31);
        line(62,29);
        
        LCD_sendC();
        break;
    case DESNO:
        clear();
        plot(0,0);
        line(42,24);
        line(42,42);
        line(0,64);
        line(42,42);
        line(84,42);
        line(128,64);
        line(84,42);
        line(84,24);
        line(128,0);
        line(84,24);
        line(42,24);
        
        plot(107,30);
        line(117,30);
        line(117,27);
        line(122,32);
        line(117,37);
        line(117,34);
        line(107,34);
        line(107,30);
        LCD_sendC();
        
        break;
    case LEVO: 
        clear();
        plot(0,0);
        line(42,24);
        line(42,42);
        line(0,64);
        line(42,42);
        line(84,42);
        line(128,64);
        line(84,42);
        line(84,24);
        line(128,0);
        line(84,24);
        line(42,24);
        
        plot(18,30);
        line(8,30);
        line(8,27);
        line(3,32);
        line(8,37);
        line(8,34);
        line(18,34);
        line(18,30);
        LCD_sendC();
        break;
    case DOL:
        clear();
        plot(0,0);
        line(42,24);
        line(42,42);
        line(0,64);
        line(42,42);
        line(84,42);
        line(128,64);
        line(84,42);
        line(84,24);
        line(128,0);
        line(84,24);
        line(42,24);
        
        plot(62,45);
        line(66,45);
        line(66,55);
        line(69,55);
        line(64,60);
        line(59,55);
        line(62,55);
        line(62,45);
        LCD_sendC();
        break;
        
        
    case GOR: 
        clear();
        plot(0,0);
        line(42,24);
        line(42,42);
        line(0,64);
        line(42,42);
        line(84,42);
        line(128,64);
        line(84,42);
        line(84,24);
        line(128,0);
        line(84,24);
        line(42,24);
        
        plot(62,19);
        line(66,19);
        line(66,9);
        line(69,9);
        line(64,4);
        line(59,9);
        line(62,9);
        line(62,19);
        LCD_sendC();
        break;
    }
    
    
    
}
/*
int prev_kable(void){
    
    spre = 0x01;
    //int sestevek = 0;
    return 1; 
    for(int i = 0; i < 8; ++i)
    {
        P4DIR = 0;  // tuki da vse na niè
        P4REN = 0xFF;
        P4OUT = 0;
        P5DIR = 0;
        P5REN = 0xFF;
        P5OUT = 0;
        
        P4DIR  = spre; // tuki jih pa poj nafila
        P4OUT = spre;
        P4REN = ~spre;
        zakasni();
        if (P4IN  != spre){// prever ce se kej stika ce kej toka stece
            
            vhod4 = P4IN;
            vhod5 = P5IN;
            spre_prvr = spre;
            keri_del = 0;
            
            return 0;
        }
        if (P5IN  != 0x20){//(spre|0x20) doda se tist za tipko
            vhod4 = P4IN;
            vhod5 = P5IN;
            spre_prvr = spre;
            keri_del = 1;
            return 0;
        }
        spre = spre << 1;
    }
    spre = 0x01;
    for(int i = 0; i < 8; ++i)
    {
        P4DIR = 0;
        P4REN = 0xFF;
        P4OUT = 0;
        P5DIR = 0;
        P5REN = 0xFF;
        P5OUT = 0;
        
        P5DIR  = spre;
        P5OUT = spre;
        P5REN = ~spre;
        
        if (P4IN  != 0){// prever ce se kej stika ce kej toka stece
            vhod4 = P4IN;
            vhod5 = P5IN;
            spre_prvr = spre;
            keri_del = 10;
            
            return 0;
        }
        if (P5IN  != (spre|0x20)){//(spre|0x20) doda se tist za tipko
            vhod4 = P4IN;
            vhod5 = P5IN;
            spre_prvr = spre;
            keri_del = 12;
            return 0;
        }
        spre = spre << 1;
    }
    Init();
    LCD_init();
    return 1;
    
}
*/




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
