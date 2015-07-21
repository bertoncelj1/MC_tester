#ifndef str_funkcije_h
#define str_funkcije_h


int strCp(char *strMain, const char *strAdd);
int strLen(const char *str);
void addNumberToStr(char *str, int st, int mesto, int vel);

#else
	#error str_funkcije  already included! 
#endif
