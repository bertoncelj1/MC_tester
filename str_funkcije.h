#ifndef str_funkcije_h
#define str_funkcije_h


int strCp(char *strMain, char *strAdd);
int strLen(char *str);
void addNumberToStr(char *str, int st, int mesto, int vel);

#else
	#error str_funkcije  already included! 
#endif
