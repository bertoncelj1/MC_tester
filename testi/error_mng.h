#ifndef error_mng_h
#define error_mng_h

#define ERR_BUFF_SIZE	100

//izpis
#define ST_ZANKOV_LINE	20 //koliko znakov bo izpisal na vrstico
#define POL_EKRANA		64
#define FONT_NASLOV		STDOUT_LCD_NORMAL_FONT
#define FONt_BESEDILO	STDOUT_LCD
#define MARGIN_TOP		5
#define MARGIN_LEFT		5
#define LINE_SPACE		8		//koliko so narazen vrstice sporocila
#define OPIS_SPACE		10		//koliko je razmika med opisom in sporocilom
#define NASLOV_SPACE	15		//koliko je razmika med naslovem in opisom

void addToErrorBuff(char * str);
void emptyErrorBuff();
void izpisiError(char *imeNapake, char *opisNapake);

#else
	#error error_mng_h  already included! 
#endif
