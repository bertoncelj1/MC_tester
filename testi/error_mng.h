#ifndef error_mng_h
#define error_mng_h

void addToErrorBuff(char * str);
void emptyErrorBuff();
int getErrorLine(char *line, int lineLen, int errorOffset);

#else
	#error error_mng_h  already included! 
#endif
