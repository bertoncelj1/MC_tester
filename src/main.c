#include <msp430.h>
#include "ini.h"
#include "graphics.h"
#include "graphics_LCD2.h"
#include "scheduler.h"
#include "timerb.h"
#include "ledice_test.h"
#include "flash_test.h"
#include "test_mng.h"
#include "run.h"
#include "main.h"

//TODO add log file

const char* getVersion(){
  return "v1.01";
}

int main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD; 
  
  //inicializacija
  Init(); 
  
  //enable interrupt
  _EINT();       
      
  //nastavi aplikacije
  initApps();
  
  while (1){
    switchApps();
  }
}


void switchApps(){
    switch( apps_poll() )
    {
    //prebere pritisnjen tipke
    case READ_KEY_APP:
      BeriKey_2();
      BeriKey(); 
      
      apps_suspend(READ_KEY_APP);
      timer_wait(READ_KEY_ID, 5);
      break;
    
    //glavna kontrolna aplikacija
    case KONTROLA_APP:  
      potek_kontrole();
      
      apps_suspend(KONTROLA_APP);
      timer_wait(KONTROLA_ID, 20);   
      break;
      
    //uporablja za testiranje ledic
    //na vsake 200ms (BLINK_SPEED) se prizge nasljednja kombinacija ledic
    case lediceToggle_APP:
      setLedNext();
      
      apps_suspend(lediceToggle_APP);        
      timer_wait(lediceToggle_ID, BLINK_SPEED); 
      break;
      
    default:
    case NO_APP:break;
  }
}


void initApps(){
  apps.active = 0;
  apps.enabled_mask = 0;
  apps_enable(REFRESH_DIS_APP | READ_KEY_APP | KONTROLA_APP | lediceToggle_APP ); 
  timer_wait(READ_KEY_ID, 10);
  timer_wait(KONTROLA_ID, 20);
}

