#include  "msp430x24x.h"
#include  "graphics.h"
#include "TIMERB.H"
#include "test_mng.h"


void Init(void)
{
  volatile unsigned int i;

  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

  BCSCTL1 &= ~XT2OFF;                       // XT2= HF XTAL

  BCSCTL2 = SELM_2 + SELS + DIVS_0;         // MCLK= XT2 (safe) SMCLK=XT2/1
  FCTL2 = FWKEY+FSSEL_2+15;     // Flash clock select: 2 - SMCLK  /16 = 460 kHz


                            
  P1DIR = 0x80+0x40+0x08+0x04+0x02+0x01; 
  P1OUT = 0;  
  P1SEL = 0x00;   
  
  P2OUT = 0x80+0x40+0x10+0x08+0x04;  
  P2DIR = 0x80+0x40+0x10+0x08+0x04+0x02+0x01; 
  P2SEL = 0x00; 
  
  P3OUT = 0x08+0x04; 
  P3DIR = 0x08+0x04;                             // All P3.x outputs
  P3SEL = 0x00;                             // P3.4,5 = UART0 TXD/RXD
  
  
  
  P4OUT = 0;   //                        // MMC CS=1
  P4DIR = 0xFF;   // LCD data               // All P4.x outputs
  P4SEL = 0;
  
  P5OUT = 0xff;                             // All P5.x set
  P5DIR = 0xDF;   // P5.5 tipka             // All P5.x outputs
  
  P6OUT = 0xff;
  P6DIR = 0x00;
  P6SEL = 0;


  TBCTL = TBSSEL_2 + ID_2 + MC_1 + TBCLR;     // SMCLK/4, up mode, CLR

  TBCCR0 = 461-1;  // 4kHz
  TBCCTL6 = OUTMOD_7;   // PWM (beep ON)
  TBCCR6 = 230;    // 50%

  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer

  
  OutDev=4;  // izhodna enota 0=LCD, 1=srednji fonti, 2=veliki fonti, 4=RS232 PC
  P4OUT=BL+CSSD; 


  //inicializacija timerB
  timerb_init();
  
  
  test_mng_init(); //inicializira test managerja, ki je odgovoren za klice testnih programov
  
  //init oba zaslona
  LCD_init();
  //LCD 2 se inicializira takrat, ko ga dejansko priklopimo
  
  led_diode(OFF);
  
}
