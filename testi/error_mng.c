#include "error_mng.h"

//v primeru da je prislo do napake, bo sporocilo napake zapisano v errorBuff
static char errorBuff[100];
static int errIndex = 0;


//doda niz str v error buffer. str se mora obcvezno koncati z 0!
void addToErrorBuff(char * str){
	errIndex = strCp(&errorBuff[errIndex], str);
}

//postavi errIndex na nic
void emptyErrorBuff(){
	errIndex = 0;
	errorBuff[0] = 0;
}

//vrne string v katerega prepise vsebino errorBufferja
//lineLen pove koliko je prstora v nizu line -> koliko elementov najvec pricakuje
//errorOffest pove s kje naj zacne brati errorBuff
//vrne 0 v primeru da ni nic vpisal v line, drugace vrne 1
int getErrorLine(char *line, int lineLen, int errorOffset){
    int i = 0;
    
    //ce v errorBuff ni vec zapisov vrne 0
    if(errorOffset >= errIndex){
      line[i] = 0;
      return 0;
    }
  
  	//dela dokler je prostor v line, ali do prvega znaka '\n', ali dokler se kaj notri pise
    while(i < lineLen && errorBuff[errorOffset + i] != '\n' && (errorOffset + i) < errIndex){
      line[i] = errorBuff[errorOffset + i];
      i++;
    }
    
    line[i] = 0;
    return 1;
}
