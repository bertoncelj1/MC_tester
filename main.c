#include <msp430.h>
#include  "graphics.h"
#include  "ini.h"
#include "head.h"
#include "scheduler.h"
#include "timerb.h"
#include "flash_test.h"
#include "ledice_test.h"
#include "test_mng.h"





int main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  Init();
  _EINT(); 
  kontrola = ZACETEK;
  
  //sahovnica_inverzno();
  LCD_init();
  
  test_mng_init();
  //prikaze moznost za izbiro tipa testiranja
  LCD_getTestniProgram();
  

  LCD_init_2();
  off_REDled;
  off_GREENled;
  flags=1;  // za risanje
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
  
  
  
  //tipPrograma = LCD_zacetnaIzbira();

  
  apps.active = 0;
  apps.enabled_mask = 0;
  apps_enable(REFRESH_DIS_APP | READ_KEY_APP | KONTROLA_APP | lediceToggle_APP );  
  timer_wait(READ_KEY_ID, 10);
  timer_wait(KONTROLA_ID, 20);
  
  
  
  
  
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
      
      apps_suspend(READ_KEY_APP);
      timer_wait(READ_KEY_ID, 5);
      break;
      
    case KONTROLA_APP:  
      potek_kontrole();
      
      apps_suspend(tipkeDVE_APP);
      timer_wait(tipkeDVE_ID,100);   
      break;
      
      
    case LCD_TEST_APP:
      
      if (tipke_2() == 1){
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
      
      
    case lediceToggle_APP:
      setLedNext();
      
      apps_suspend(lediceToggle_APP);        
      timer_wait(lediceToggle_ID, 300); 
      
      break;

  }
  
  
}

