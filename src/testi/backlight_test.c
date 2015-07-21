#include "backlight_test.h"
#include "graphics.h"
#include "error_mng.h"

void delay(unsigned int d);

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




