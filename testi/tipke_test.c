#include "tipke_test.h"
#include "graphics.h"

static int display_state;

void izpis_pritisnjenih_tipk(void);
void key_error(int t);

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
    clear_2();
    break;
  }
  
  LCD_sendC_2();
}

void  izpis_pritisnjenih_tipk(void){
  clearLine(2,8);
  OutDev = STDOUT_LCD_NORMAL_FONT; 
  
  if(test_tipk_biti & 0x08){
    GrX =90;  GrY = 15;
    printf("%s","+");
  }
  if(test_tipk_biti & 0x10){
    GrX =90;  GrY = 50;
    printf("%s","-");
  }
  if(test_tipk_biti & 0x04){
    GrX =60;  GrY = 30;
    printf("%s","<");
  }
  if(test_tipk_biti & 0x02){
    GrX =120;  GrY = 30;
    printf("%s",">");
  }
  if(test_tipk_biti & 0x01){
    GrX =83;  GrY = 30;
    printf("%s","OK");
  }
  LCD_sendC();
}

void key_error(int t){
  clear();
  OutDev = STDOUT_LCD_NORMAL_FONT; 
  GrX =20;  GrY = 5;
  printf("%s","NAPAKA !!!");
  GrX =5;  GrY = 25; 
  printf("%s","HKRATI PRITISN.");
  GrX = 3 ;  GrY = 45;
  printf("TIPKE:" );
  //(TkGor+TkDol+TkLev+TkDes+TkEnt)
  if ( t & TkEnt){
    printf(" OK," );
  }
  if ( t & TkGor){
    printf(" +," );
  }
  if ( t & TkDol){
    printf(" -," );
  }
  if ( t & TkLev){
    printf(" <," );
  }
  if ( t & TkDes){
    printf(" >," );
  }
  //ponovi test
  LCD_sendC();
}


int tipke_2(void){
  int tipka;
  
  //TODO dodej da bo vsak test mu svojo init funkcijo
  static int prvic = 0;
  if(!prvic){
    LCD_init_2();
    prvic = 1;
  }
  
  
  
  tipka=KeyBuf_2;
  KeyBuf_2 = 0;
  //(TkGor+TkDol+TkLev+TkDes+TkEnt)
  if(tipka){
    switch (tipka){
    case TkEnt: //enter
      if(test_tipk_biti & 0x01){
      }
      else{
        display_state++;
        test_tipk_biti |= 0x01;
      }
      break;
      
    case TkDes: //Desno
      if(test_tipk_biti & 0x02){
      }
      else{
        test_tipk_biti |= 0x02;
        display_state++;
      }
      
      break;
      
    case TkLev: //Levo
      if(test_tipk_biti & 0x04){
      }
      else{
        test_tipk_biti |= 0x04;
        display_state++;
      }
      break;
      
    case TkGor: //Gor
      if(test_tipk_biti & 0x08){
      }
      else{
        test_tipk_biti |= 0x08;
        display_state++;
      }
      break; 
      
    case TkDol: //Dol  
      if(test_tipk_biti & 0x10){
      }
      else{
        test_tipk_biti |= 0x10;
        display_state++;
      }
      break;
      
    default:
      
      //TODO popravi da bo to boljse delalo
      // napaka
      key_error(tipka);
      return -1;
      break;
      
    }
    display_test();
    izpis_pritisnjenih_tipk();
    
    if(test_tipk_biti == 0x1F){
      test_tipk_biti = 0;
      display_state = 0;
      prvic = 0;
      return 1;
    }
    else{
      return 0;
    }
  }
  
  
  return 0;
}


