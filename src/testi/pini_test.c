#include "pini_test.h"
#include "error_mng.h"
#include "graphics.h"
#include "graphics_LCD2.h"

// pin 3 tipka, 4 GND in 5 Vcc se izpustijo
char testP1[6] = {0x01,0x02,0x04,0x08,0x40,0x80};
char pinP1[6]  = { 11,  13,  15,  17,  18,  16 };
char testP2[7] = {0x01,0x02,0x04,0x08,0x10,0x40,0x80};
char pinP2[7]  = { 14,  12,  10,  8,   6,   2,   1 };
char testP3[3] = {0x04,0x08};
char pinP3[3]  = {  7 ,  9 };

//zapise napacin string v error 
void writePInToError(char pin){
	char error[4];
	int stLen = (pin < 10)? 1 : 2;
               
        int i = stLen - 1; //kam bo zapisal desni del stevila, (azpisuje iz desnega proti levi)
        while(pin > 0){
          error[i--] =  pin % 10 + '0';
          pin /= 10;
        }
        
        error[stLen++] = ' ';
        error[stLen++] = 0;
	
	addToErrorBuff(error);
}

int prev_pine(void){
	emptyErrorBuff();//sprazne errotBuffer in ga pripravi za zapis
    
    char P1,P2,P3;
    char pin;
    
    P1 = 0x01 | 0x02 | 0x04 | 0x08 | 0x40 | 0x80;
    P2 = 0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x40 | 0x80;
    P3 = 0x01 | 0x04 | 0x08;
    
    //zapovni si vrednosti registrov
    char m_P1DIR = P1DIR;
    char m_P1OUT = P1OUT;
    char m_P1REN = P1REN;
    
    char m_P2DIR = P2DIR;
    char m_P2OUT = P2OUT;
    char m_P2REN = P2REN;
    
    char m_P3DIR = P3DIR;
    char m_P3OUT = P3OUT;
    char m_P3REN = P3REN;
    
    int vRedu = 1;
    
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
        	 writePInToError(pinP1[i]);
        	 vRedu = 0;
        }
        pin = P2IN & ~(0x20);
        if (pin  != 0){
			writePInToError(pinP1[i]);
			vRedu = 0;
        }
        pin = P3IN & ~(0x01 | 0x02 | 0x10 | 0x20 | 0x40 | 0x80);
        if (pin  != 0){// preskoèi se tipka
			writePInToError(pinP1[i]);
			vRedu = 0;
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
            writePInToError(pinP2[i]);
            vRedu = 0;
        }
        pin = P2IN & ~(testP2[i] | 0x20);
        if (pin  != 0){
            writePInToError(pinP2[i]);
            vRedu = 0;
        }
        pin = P3IN & ~(0x01 | 0x02 | 0x10 | 0x20 | 0x40 | 0x80);
        if (pin  != 0){// preskoci se tipka
            writePInToError(pinP2[i]);
            vRedu = 0;
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
            writePInToError(pinP3[i]);
            vRedu = 0;
        }
        
        pin = P2IN & ~(0x20);
        if (pin  != 0){
            writePInToError(pinP3[i]);
            vRedu = 0;
        }
        
        pin = P3IN & ~(testP3[i] | 0x01 | 0x02 | 0x10 | 0x20 | 0x40 | 0x80);
        if (pin  != 0){// preskoci se tipka
            writePInToError(pinP3[i]);
            vRedu = 0;
        }
    }
    
    //ponastavi vrednosti rtegistrov
    P1DIR = m_P1DIR;
    P1OUT = m_P1OUT;
    P1REN = m_P1REN;
    
    P2DIR = m_P2DIR;
    P2OUT = m_P2OUT;
    P2REN = m_P2REN;
    
    P3DIR = m_P3DIR;
    P3OUT = m_P3OUT;
    P3REN = m_P3REN;
    
    return vRedu;
    
}

void zakasni(void){
    for(int i = 0; i < 200; ++i);
}
