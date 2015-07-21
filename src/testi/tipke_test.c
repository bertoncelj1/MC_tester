#include "tipke_test.h"
#include "graphics.h"
#include "error_mng.h"

static int display_state;

void izpis_pritisnjenih_tipk(void);
void key_error(int t);

char vse = (TkGor | TkDol | TkLev | TkDes | TkEnt);//vse tipke ki se testirajo

static int prvic = 0;
void tipke_test_init(){
      test_tipk_biti = 0;
      display_state = 0;
      prvic = 0;
}

void display_test(){
  clear_2();
  
  switch( display_state )
  {
  case 1:
    back_light_ON();
    zgoraj_belo();
    break;
    
  case 2:
    back_light_OFF();
    zgoraj_crno();
    break;
  case 3:
    back_light_ON();
    sahovnica();
    break;
    
  case 4:
    back_light_OFF();
    sahovnica_inverzno();
    break;
    
  case 5:
    back_light_ON();
    zgoraj_belo();
    zgoraj_crno();
    break;
  }
  
  LCD_sendC_2();
}

void  izpis_pritisnjenih_tipk(void){
  clearLine(2,8);
  OutDev = STDOUT_LCD_NORMAL_FONT; 
  
  int sirEkr = 128;
  int visEkr = 64;
  
  if(test_tipk_biti & TkGor){
    GrX =sirEkr/2;  GrY = visEkr/4;
    printf("%s","+");
  }
  if(test_tipk_biti & TkDol){
    GrX =sirEkr/2;  GrY = visEkr - visEkr/4;
    printf("%s","-");
  }
  if(test_tipk_biti & TkLev){
    GrX =sirEkr / 4;  GrY = visEkr/2;
    printf("%s","<");
  }
  if(test_tipk_biti & TkDes){
    GrX =sirEkr - sirEkr / 4;  GrY = visEkr/2;
    printf("%s",">");
  }
  if(test_tipk_biti & TkEnt){
    GrX =sirEkr/2-8;  GrY = visEkr/2;
    printf("%s","OK");
  }
  LCD_sendC();
}

void key_error(int t){
  emptyErrorBuff();
  
  //v error buffer sharni tipke ki so bile pritsnjene hkrati
  if ( t & TkEnt){
    addToErrorBuff("OK ");
  }
  if ( t & TkGor){
    addToErrorBuff(" + ");
  }
  if ( t & TkDol){
    addToErrorBuff(" - ");
  }
  if ( t & TkLev){
    addToErrorBuff(" < ");
  }
  if ( t & TkDes){
    addToErrorBuff(" > ");
  }
  
  tipke_test_init();
}


int tipke_2(void){
  int tipka;
  
  //TODO dodej da bo vsak test mu svojo init funkcijo
  if(!prvic){
    LCD_init_2();
    prvic = 1;
  }
  
  tipka=KeyBuf_2;
  KeyBuf_2 = 0;
  
  
  
  if(tipka){
    //odstrani vse tipke ki so bile ze pritisnjene
    tipka &= ~test_tipk_biti;
    tipka &= vse;
    
    int i;
    int st = 0;
    for(i=0x80; i > 0; i>>=1){
      //pogleda ce obstaja tipka v naboru vseh tipk in ce je bila pritisnjena
      if(i&vse && tipka&i){
        test_tipk_biti |= i;
        st ++;
        display_state++;
      }
    }
    
    if(st > 1){
      key_error(tipka);
      return -1;
    }
    
    display_test();
    izpis_pritisnjenih_tipk();
    
    if(test_tipk_biti == vse){
      tipke_test_init();
      return 1;
    }
    else{
      return 0;
    }
  }
  
  
  return 0;
}


