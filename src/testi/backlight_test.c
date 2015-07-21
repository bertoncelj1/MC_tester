#include "backlight_test.h"
#include "graphics.h"
#include "error_mng.h"


void delay(unsigned int d);
void init_back_light();
int test_back_light(void);



s_testnaOperacija backlight_test = {
  "backlight",
  init_back_light,
  preveri_backlight,
};

void* getTstOprBacklight(){
  return &backlight_test;
}

void init_back_light(){

}

e_OprState preveri_backlight(){
  if(test_back_light()){
    return OPR_KONCANA;
  }
  
  //v primeru napake izpise obvestilo
  izpisiError("NAPAKA BACKLIGHT", "opis napake:");
  
  return OPR_NAPAKA;
}

int test_back_light(void){
    int vRedu = 1;
    if(P3OUT & 0x02 ){ //preveri ali je Backlight vkljucen; potem lahko vkljucim test
        
        P2DIR &= ~ 0x80;  // P2.7 ali je napetost 3-4V
        P2DIR &= ~ 0x40;  // P2.6 ali je tok veèji od 5mA
        
        delay(100);
        
        emptyErrorBuff();
        if(!(P2IN & 0x80 )){
            //napetost ni dovolj velika
            addToErrorBuff("- premajna napetost\n");
            vRedu = 0;
        }
        if(!(P2IN & 0x40 )){
            //tok ni dovolj velik
            addToErrorBuff("- premajhen tok\n");
            vRedu = 0;
        }
    }
  
    
    return vRedu;
}
void delay(unsigned int d){
  while(d--);
}




