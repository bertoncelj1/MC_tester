#include "tipke_test.h"
#include "graphics.h"
#include "graphics_LCD2.h"
#include "error_mng.h"

static int display_state;

void izpis_pritisnjenih_tipk(void);
void key_error(int t);
void display_test();

char naborTipk = (TkGor | TkDol | TkLev | TkDes | TkEnt);//vse tipke ki se testirajo

//TODO pobrisi tisti del ekrana na katerega bos risal
//TODO izpisi navodila ... pac da mora zacet pritiskat tipke
void tipke_test_init(){
      LCD_init_2();
      test_tipk_biti = 0;
      display_state = 0;
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
  OutDev = STDOUT_LCD_NORMAL_FONT; 
  
  int sirEkr = 128;
  int visEkr = 64;
  
  //postvavi izris za tipke v podane meje:
  int x1 = sirEkr/2+10;
  int y1 = 16;
  int x2 = sirEkr;
  int y2 = visEkr;
  int velPis = 8; //velikost pisave
  
  
  if(test_tipk_biti & TkGor){
    GrX = x1 +(x2-x1-velPis)/2;  GrY = y1;
    printf("%s","+");
  }
  if(test_tipk_biti & TkDol){
    GrX =x1 +(x2-x1-velPis)/2;  GrY = y2 - velPis;
    printf("%s","-");
  }
  if(test_tipk_biti & TkLev){
    GrX =x1;  GrY = y1 + (y2-y1-velPis)/2;
    printf("%s","<");
  }
  if(test_tipk_biti & TkDes){
    GrX =x2 - velPis;  GrY =  y1 + (y2-y1-velPis)/2;
    printf("%s",">");
  }
  if(test_tipk_biti & TkEnt){
    GrX =x1 +(x2-x1-velPis*2)/2;  GrY =  y1 + (y2-y1-velPis)/2;
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
  
  tipka=KeyBuf_2;
  KeyBuf_2 = 0;
  
  if(tipka){
    //odstrani vse tipke ki so bile ze pritisnjene
    tipka &= ~test_tipk_biti;
    //gleda samo tiste tike ki so predpisane v vse
    tipka &= naborTipk;
    
    int i;
    int st = 0;
    for(i=0x80; i > 0; i>>=1){
      //pogleda ce obstaja tipka v naboru vseh tipk in ce je bila pritisnjena
      if(i&naborTipk && tipka&i){
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
    
    //1-test tipke koncan, 0-se v teku
    return (test_tipk_biti == naborTipk);
  }
  
  return 0;
}


