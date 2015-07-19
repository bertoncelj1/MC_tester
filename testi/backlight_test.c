#include "backlight_test.h"
#include "graphics.h"


int test_back_light(void){
  
    int vRedu = 1;
    if( P3OUT & 0x02 ){ //preveri ali je Backlight vkljucen; potem lahko vkljucim test
        
        P2DIR &= ~ 0x80;  // P2.7 ali je napetost 3-4V
        P2DIR &= ~ 0x40;  // P2.6 ali je tok veèji od 5mA
        
        
        emptyErrorBuff();
        if(!(P2IN & 0x80 )){
            //napetost ni dovolj velika
            addToErrorBuff("Premajna Napetost\n");
            vRedu = 0;
        }
        if(!(P2IN & 0x40 )){
            //tok ni dovolj velik
            addToErrorBuff("Premajhen Tok\n");
            vRedu = 0;
        }
    }
 
    return vRedu;
}





