#include <msp430.h>
#include "graphics.h"
#include "ini.h"
#include "head.h"
#include "scheduler.h"
#include "timerb.h"
#include "ledice_test.h"
#include "flash_test.h"
#include "test_mng.h"



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
    //na vsake 300ms se prizge nasljednja kombinacija ledic
    case lediceToggle_APP:
      setLedNext();
      
      apps_suspend(lediceToggle_APP);        
      timer_wait(lediceToggle_ID, 300); 
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

