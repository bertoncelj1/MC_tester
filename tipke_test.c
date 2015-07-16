#include "tipke_test.h"

static int display_state;


void izrisi_tipko(int tip);
void  izpis_pritisnjenih_tipk(void);

void izrisi_tipko(int tip){
    
    switch (tip){
    case ENTER:
        clear();
        plot(0,0);
		line(42,24);line(42,42);line(0,64);line(42,42);line(84,42);line(128,64);
		line(84,42);line(84,24);line(128,0);line(84,24);line(42,24);
        
        plot(62,29);
		line(65,29);line(67,31);line(67,34);line(65,36);line(62,36);line(60,34);
		line(60,31);line(62,29);
        
        LCD_sendC();
        break;
		
    case DESNO:
        clear();
        plot(0,0);
		line(42,24);line(42,42);line(0,64);line(42,42);line(84,42);line(128,64);
		line(84,42);line(84,24);line(128,0);line(84,24);line(42,24);
        
        plot(107,30);
		line(117,30);line(117,27);line(122,32);line(117,37);line(117,34);line(107,34);
		line(107,30);
		
        LCD_sendC();
        break;
		
    case LEVO: 
        clear();
        plot(0,0);
		line(42,24);line(42,42);line(0,64);line(42,42);line(84,42);line(128,64);
		line(84,42);line(84,24);line(128,0);line(84,24);line(42,24);
        
        plot(18,30);
		line(8,30);line(8,27);line(3,32);line(8,37);line(8,34);line(18,34);line(18,30);
		
        LCD_sendC();
        break;
		
    case DOL:
        clear();
        plot(0,0);
		line(42,24);line(42,42);line(0,64);line(42,42);line(84,42);line(128,64);
		line(84,42);line(84,24);line(128,0);line(84,24);line(42,24);
        
        plot(62,45);
		line(66,45);line(66,55);line(69,55);line(64,60);line(59,55);line(62,55);
		line(62,45);
		
        LCD_sendC();
        break;
        
        
    case GOR: 
        clear();
		plot(0,0);
		line(42,24);line(42,42);line(0,64);line(42,42);line(84,42);line(128,64);
		line(84,42);line(84,24);line(128,0);line(84,24);line(42,24);
        
        plot(62,19);
		line(66,19);line(66,9);line(69,9);line(64,4);line(59,9);line(62,9);line(62,19);
		
        LCD_sendC();
        break;
    }
    
}

void display_test(){
    clear();
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
        zgoraj_crno();
        zgoraj_belo();
        
        break;
    }
   
    LCD_sendC_2();
}

void  izpis_pritisnjenih_tipk(void){
    clear();
    OutDev = STDOUT_LCD_NORMAL_FONT; 
    
    if(test_tipk_biti & 0x08){
        GrX =90;  GrY = 5;
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
      apps_disable(LCD_TEST_APP); 
      apps_enable(tipkeDVE_APP);
      timer_wait(tipkeDVE_ID,200);
}


int tipke_2(void){
    int t;
    
    t=KeyBuf_2[0];
    KeyBuf_2[0] = 0;
    //(TkGor+TkDol+TkLev+TkDes+TkEnt)
    if(t){
        switch (t){
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
            
            // napaka
            key_error(t);
            return 0;
            break;
            
        }
        display_test();
        izpis_pritisnjenih_tipk();
        
        if (test_back_light()){
           //back light ne dela dobro
           back_light_error(tmp); 
           return 0; 
        }
        
        if(test_tipk_biti == 0x1F){
          return 1;
        }
        else{
          return 0;
        }
    }
    
    
    return 0;
}


