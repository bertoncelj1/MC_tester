#include "msp430x24x.h"
#include "ledice_test.h"
#include "scheduler.h"



s_testnaOperacija ledice_test = {
  "ledice",
  ledice_test_init,
  preveriLedice,
};

void* getTstOprLedice(){
  return &ledice_test;
}

void ledice_test_init(){
	//postavi vse na pine na out
	dir_ledAlert_1;
	dir_ledPulse_1;
	dir_latch_1;
}

e_OprState preveriLedice(){
  //poklice se app za testiranje ledic
  //opercije je vedno uspesa saj uporabnik sam preveri ledice
  timer_wait(lediceToggle_ID, 50);
  return 1;
}

//sprejme stevilo od 0 do 3 in glede na binarno vrednost tega stevila prizge ledici
void setLed(int prizgane){
  set_latch_1;
  if(prizgane & 0x01){
    set_ledPulse_1;
  }else{
    set_ledPulse_0;
  }
  
  if(prizgane & 0x02){
    set_ledAlert_0;
  }else{
    set_ledAlert_1;
  }
  set_latch_0;
  
}

//prizge nasljednjo konfiguracijo ledic
void setLedNext(){
  static int roll = 0;
  setLed(roll ++);
}
