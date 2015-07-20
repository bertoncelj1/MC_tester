#include "error_mng.h"
#include "str_funkcije.h"
#include "graphics.h"

//v primeru da je prislo do napake, bo sporocilo napake zapisano v errorBuff
static char errorBuff[ERR_BUFF_SIZE];
static int errIndex = 0;


//doda niz str v error buffer. str se mora obcvezno koncati z 0!
void addToErrorBuff(char * str){
	errIndex += strCp(&errorBuff[errIndex], str);
}

//postavi errIndex na nic
void emptyErrorBuff(){
	errIndex = 0;
	errorBuff[0] = '\0';
}


//iz test_mng errof bufferja prebere npako in jo izpise
//to napako naj bi tja zapisal ze sam proces
void izpisiError(char *imeNapake, char *opisNapake){
  clearLine(2, 8);

  int lineY = MARGIN_TOP;	//trenutna pozicija izpisa za Y os

  //izpise naslov na sredino horizontalno
  OutDev = STDOUT_LCD_NORMAL_FONT;
  GrX = POL_EKRANA - strLen(imeNapake)*FONT_WIDTH/2;  GrY = lineY;
  printf("%s", imeNapake);
  lineY += NASLOV_SPACE;
  
  //izpise opis napake
  OutDev = STDOUT_LCD;
  
  GrX = MARGIN_LEFT-3;  GrY = lineY;
  printf("%s", opisNapake);
  lineY += OPIS_SPACE;
  
  //izpise vrstice
  int i=0;
  int zacLine; //pove kje se vrstica zacne
  while(i < errIndex){
  
  	zacLine = i;
  	//poisce simbole ki zacnejo novo vrstico
  	while(i-zacLine < ST_CRK_NA_VRSTO && errorBuff[i] != '\n' && i < errIndex){
  		i++;
  	}
  	//trenutni znak zamenja z 0 zato da bo printf znal pravilno izpisati 
  	char treChar = errorBuff[i];
  	errorBuff[i] = '\0';
  	 
  	GrX = MARGIN_LEFT;  GrY = lineY;
  	printf("%s", &errorBuff[zacLine]);
  	lineY += LINE_SPACE;
  	
  	errorBuff[i] = treChar;
  	
  	//ce je nasel znak \n mora povecati i zato da ga v nasljednji iritaciji izspusti
  	if(errorBuff[i] == '\n')i++;
  }
  
  
  LCD_sendC();
}

