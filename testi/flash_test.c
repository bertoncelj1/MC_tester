#include  "msp430x24x.h"
#include  "flash_test.h"


void flash_test_init(){
	//sets pins directions, 1->pin out, 0->pin input
	dir_clock_1;
	dir_input_1;
	dir_output_0;
	dir_chipSel_1;
}

//prebere ID od flasha in vrne atribute, ki mu jih podamo
//manId -> manufacturer ID      1Byte
//devID -> device ID            2Byta
//uniqueID          pointer na string
//uniqueLen         dolzina rezarviranega prostora v uniqueID. Po koncani operacii notri zapise dolzino prebranega 
uint8_t readId(uint8_t *manID, uint16_t *devID, uint8_t *uniqueID, uint8_t *uniqueLen){
	//start operation
        set_clock_0;
	set_chipSel_0;
        
	//POSLJE ukaz
	sendByte(CMD_READ_ID);

	//BERE
	//prvi byte je manufacturer ID
	*manID = readByte();            //pricakuje 0x20
	
	//druga dva byta sta device ID
	uint16_t prvi = 0;
	uint16_t drugi = 0;
	prvi = readByte();      //memory type, pricakuje 0xBA
	drugi = readByte();     //memory capacity pricakuje 0x17 (64Mb)
	*devID = (prvi & 0xff) + ((drugi & 0xff) << 8);
	
	//ostali byte predstavljajo unique ID
	uint8_t len = readByte(); //najprej prebere dolzino unique ID-ja
	uint8_t i;
	for(i=0; i<len; i++){
	  if(i >= *uniqueLen)return 0; //napaka, premalo prostora v uniqueID
	  uniqueID[i] = readByte();
	}
	*uniqueLen = len;        //nastavi uniqueLen, da bo uporabnik vedel koliko je dolg uniqueID string
        	
	//end operation
	set_chipSel_1;
        
    return 1; 
}



//preveri ali flash deluje tako da poklice readId funkcijo in preveri vrnjene komponente
//v primero napake, zapise v errorBuff sporocilo, kaj je so narobe
uint8_t preveriFlash(){
	uint8_t manID = 0;
	uint16_t devID = 0;
	uint8_t uniqueID[20];
	uint8_t uniqueLen = 20;
	
	flash_test_init(); // ponastavi pine za vsak slucaj, ce je to uporabnik pozabil
	int vRedu = 1;
        if(!readId(&manID, &devID, uniqueID, &uniqueLen))vRedu = 0;
	
	//preveri vrednosti ki jih je vrnila funkcija readId()
	if(manID != 0x20)vRedu = 0;
	else if(devID != 0x17BA)vRedu = 0;
	else if(uniqueLen != 0x10)vRedu = 0;
	
        //ce je prislo do napake zapise sporocilo v errorBuff
        //zapise vse podatke v errorBuff sesnajstisko
        if(!vRedu){
          char error[50];
          char errIndex = 0;
          //nabor sesnajstiskih znakov
          uint8_t nabor16x[17] = "0123456789ABCDEF";
          //manID
          error[errIndex++] = nabor16x[(manID >> 4)& 0x0F];
          error[errIndex++] = nabor16x[(manID & 0x0F)];
          error[errIndex++] = ' ';
          
          //devID
          error[errIndex++] = nabor16x[(devID >> 12)& 0x0F];
          error[errIndex++] = nabor16x[(devID >> 8)& 0x0F];
          error[errIndex++] = nabor16x[(devID >> 4)& 0x0F];
          error[errIndex++] = nabor16x[(devID & 0x0F)];
          error[errIndex++] = ' ';
          
          //uniqueLen
          error[errIndex++] = nabor16x[(uniqueLen >> 4)& 0x0F];
          error[errIndex++] = nabor16x[(uniqueLen & 0x0F)];
          
          int i;
          for(i=0; i<uniqueLen; i++){
            error[errIndex++] = nabor16x[(uniqueID[i] >> 4)& 0x0F];
            error[errIndex++] = nabor16x[(uniqueID[i] & 0x0F)];
          }
          
          error[errIndex++] = 0;
          
          emptyErrorBuff();
          addToErrorBuff(error);
          
        }
        
	return vRedu;
}


//poslje Byte flashu. Posilja ga bit po bit iz leve proti desni, MSB first
void sendByte(uint8_t byte){
	int i;
        
	for(i = 1<<7; i>0; i >>= 1){      
		
		//ce je bit na i-te mestu nastavi output na 1 drugace na 0
		if(i & byte)set_input_1;
		else set_input_0;
		
		set_clock_1;
		set_clock_0;
	}
}

//prebere Byte
uint8_t readByte(){
	uint8_t prebrano = 0;
	int i;
	for(i = 1<<7; i>0; i >>= 1){
		set_clock_0;
		
		// ce je get_input postavi bit na mesto kjer je trenutno i
		if(get_output)prebrano |= i;            
        
		set_clock_1;
	}
	
	return prebrano;
}




