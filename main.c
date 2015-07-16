#include <msp430.h>
#include  "graphics.h"
#include  "ini.h"
#include "head.h"
#include "scheduler.h"
#include "timerb.h"
#include "flash_test.h"
#include "ledice_test.h"


izbira izbira1 = {
  "prog flash",
  123,
};

izbira izbira2 = {
  "brez",
  123,
};

izbira *izbire[10];



char strProgFlas[] = "prog flash";
char strProgNorm[] = "brez";

int strLen(char *str){
  int len = 0;
  while(*str++)len ++;
  return len;
}
void izpisiIzbiro(char *str){
  int sir = 132;
  int margin = 5;
  int sirNormFonta = 6;
  int sirLCDFonta = 6;
  int visinaIzpisa = 50;
  int sirBesedila = strLen(str)*sirLCDFonta;
  
  OutDev = STDOUT_LCD_NORMAL_FONT;
   
  GrX = sir/2-sirBesedila/2-margin-sirNormFonta;  
  GrY = visinaIzpisa; 
  printf("<");
  
  GrX = sir/2+sirBesedila/2+margin;  
  GrY = visinaIzpisa; 
  printf(">");
  
  OutDev = STDOUT_LCD;
  GrX = sir/2 - sirBesedila/2;
  GrY = visinaIzpisa + 1; 
  printf(str);
  
  LCD_sendC();
}

int LCD_zacetnaIzbira(){
  clear();
  OutDev = STDOUT_LCD; 
  GrX = 20;  GrY = 5;
  printf("Izberite tip");
    
  GrX = 20;  GrY = 15; 
  printf("naprave, ki jo");
  
  GrX = 20;  GrY = 25; 
  printf("zelite testirati");
  
  LCD_sendC();
  
  int trenutnaIzbira = PROG_FLASH;
  while(1){
    if(1){//(tipkaLevo()){
      if(--trenutnaIzbira <= 0)trenutnaIzbira = 123 - 1;
      
    }else if(1){//(tipkaDesno()){
      trenutnaIzbira++;
      trenutnaIzbira %= 123;
      
    }else if(1){//(tipkaOK()){
      return trenutnaIzbira;
    }
    izpisiIzbiro(izbire[trenutnaIzbira]->ime);
    
  }
}

int tipPrograma = PROG_NORM;

int main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  Init();
  _EINT(); 
  kontrola = ZACETEK;
  
  //sahovnica_inverzno();
  LCD_init();
  
  izbire[PROG_FLASH] = &izbira1;
  izbire[PROG_NORM] = &izbira2;
  
  
  LCD_init_2();
  off_REDled;
  off_GREENled;
  flags=1;  // za risanje
  LCDpr=1;
  blink = 0;
  zakasnitev_ser_mode = 0;
  
  
  //ugasne back light
  //pri MC330 se krmili tranzistor 
  P4LATCH = 0x00;
  LE573set();
  LE573hold();
  
  //ugasne back light
  P4LATCH2 = 0x00;
  LE573set_2();
  LE573hold_2();
  
  napaka = 0;
  LCD_vstavljen = 0;
  test_koncan = 0;
  
  
  //tipPrograma = LCD_zacetnaIzbira();
  /*
  do{
  clear();
  //LCD_sendC_2();
  
  sahovnica();
  LCD_sendC_2();
  __delay_cycles(500000);
  
  clear();
  sahovnica_inverzno();
  LCD_sendC_2();
  __delay_cycles(500000);
  
  
}
  while(1);
  */
  
  apps.active = 0;
  apps.enabled_mask = 0;
  apps_enable(REFRESH_DIS_APP | REC_APP | READ_KEY_APP | tipkeDVE_APP | lediceToggle_APP );  
  //timer_wait(REFRESH_DIS_ID, 100);
  timer_wait(READ_KEY_ID, 10);
  timer_wait(tipkeDVE_ID, 20);
  //timer_wait(lediceToggle_ID, 50);
  
  
  
  
  
  while (1)
  {
    
    switch( apps_poll() )
    {
    default:
    case NO_APP:
      
      break;
      
    case READ_KEY_APP:
      BeriKey_2();
      BeriKey();
      led_diode();
      apps_suspend(READ_KEY_APP);
      timer_wait(READ_KEY_ID, 5);
      
      break;
      
    case tipkeDVE_APP:
      
      potek_kontrole();
      apps_suspend(tipkeDVE_APP);
      timer_wait(tipkeDVE_ID,100);
      //}else {
      
      //}
      
      break;
    case LCD_TEST_APP:
      
      //display_test();
      if (tipke_2() == 1){
        napaka = 0;
        LCD_vstavljen = 1;
        test_koncan = 1;
        
        LCD_init_2();
        
        //ugasne backlight
        P4LATCH2 = 0x00;
        LE573set_2();
        LE573hold_2();
        
        OutDev = STDOUT_LCD_NORMAL_FONT; 
        GrX =2;  GrY = 12;
        printf("%s","TEST");
        GrX =2;  GrY = 35; 
        printf("%s","KONCAN");
        LCD_sendC();
        
        KeyBuf_2[0]=KeyBuf_2[1]=0; 
        apps_suspend(LCD_TEST_APP);
        apps_disable(LCD_TEST_APP);
        
        
      }
      else{
        apps_suspend(LCD_TEST_APP);
        timer_wait(LCD_TEST_ID, 20);
      }
      break; 
      
      
    case REFRESH_DIS_APP:
      break;
      
    case MENU_APP:
      break;
      
    case AUX_TX_APP:
      break;     
      
    case SET_GAIN_APP:
      break;
      
    case REC_APP:
      break;
      
      
      
    case WRITE_TO_FLASH_APP:
      
      //apps_suspend(WRITE_TO_FLASH_APP);
      //timer_wait(REFRESH_DIS_ID, 300);
      break;
    case lediceToggle_APP:
      setLedNext();
      
      apps_suspend(lediceToggle_APP);        
      timer_wait(lediceToggle_ID, 300); 
      
      break;
    case a6_APP:
      
      break;
      
    case a7_APP:
      
      break;
      
    case a8_APP:
      
      break;
      
    }
  }
  
  
}

