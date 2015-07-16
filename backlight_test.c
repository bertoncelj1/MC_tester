#include "backlight_test.h"

//v primeru da je prislo do napake bo sporocilo napake zapisano v errorBuff
static char errorBuff[100];
static int errIndex = 0;

//kopira celoten strAdd v strMain, vrne dolzino strAdd
int strCp(char *strMain, char *strAdd){
	int len = 0;
	while(*strAdd){
		*strMain++ = *strAdd++;
		len ++;
	}
	return len;
}

//doda niz str v error buffer
void addToErrorBuff(char * str){
	errIndex = strCp(&errorBuff[errIndex], str);
}

int test_back_light(void){
  
    if( P3OUT & 0x02 ){ //preveri ali je Backlight vkljuèen potem lahko vkljuèim test
        
        P2DIR &= ~ 0x80;  // P2.7 ali je napetost 3-4V
        P2DIR &= ~ 0x40;  // P2.6 ali je tok veèji od 5mA
        
        errIndex = 0;
        if(!(P2IN & 0x80 )){
        	//napetost ni dovolj velika
        	addToErrorBuff("Premajna Napetost   ");
        }
        if(!(P2IN & 0x40 )){
            //tok ni dovolj velik
            addToErrorBuff("Premajhen Tok");
        }
    }
 
    return 0;
}

//TODO: dodaj da bo zaznaval \n v errorBuff in tako vrnil dejansko vrstico
//vrne string v katerega prepise vsebino errorBufferja
//lineLen pove koliko je prstora v nizu line -> koliko elementov najvec pricakuje
//errorOffest pove s kje naj zacne brati errorBuff
//vrne 0 v primeru da ni nic vpisal v line, drugace vrne 1
uint8_t getErrorBacklightLine(uint8_t *line, uint8_t lineLen, uint8_t errorOffset){
    int i = 0;
    
    //ce v errorBuff ni vec zapisov vrne 0
    if(errorOffset >= errIndex){
      line[i] = 0;
      return 0;
    }
  
    while(i < lineLen && (errorOffset + i) < errIndex){
      line[i] = errorBuff[errorOffset + i];
      i++;
    }
    
    line[i] = 0;
    return 1;
}

void back_light_error(int t){

      clear();
      OutDev = STDOUT_LCD_NORMAL_FONT; 
      GrX =25;  GrY = 5;
      printf("NAPAKA !!!");
      GrX =15;  GrY = 25; 
      printf("BACK LIGHT");
      GrX = 25 ;  GrY = 45;
      
      if(t == 2){
        printf("TOK");
      }
      else{
        printf("NAPETOST" );  
      }
      
      LCD_sendC();
      apps_disable(LCD_TEST_APP); 
      apps_enable(tipkeDVE_APP);
      timer_wait(tipkeDVE_ID,200);
}

