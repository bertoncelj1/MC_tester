


//kopira celoten strAdd v strMain, vrne dolzino strAdd
int strCp(char *strMain, char *strAdd){
	int len = 0;
	while(*strAdd){
		*strMain++ = *strAdd++;
		len ++;
	}
	return len;
}

int strLen(char *str){
  int len = 0;
  while(*str++)len ++;
  return len;
}

//podanemu stringu "str" doda stevilo na podano mesto z desno poravnavo
//ce je vel vecja od dolzine stevila na preostala mesta zapise nicle Npr:
//str = "to je str"; mesto = 3; vel = 4 st = 12; -> str = "to 0012tr"
void addNumberToStr(char *str, int st, int mesto, int vel){
  int i;
  //pomika se po stringu, od desne proti levi
  for(i=mesto+vel-1; i >= mesto; i--){
    str[i] = (st % 10) + '0';
    st /= 10;
  }
}
