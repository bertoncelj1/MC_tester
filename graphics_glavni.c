#include  "graphics.h"


unsigned char P4LATCH2;  // stanje za LATCH 74HC573
unsigned char DelayKey_2;  // zakasnitev ponavljanja tipk
unsigned char LastKey_2;   // zadnja kombinacija tipk
unsigned char KeyBuf_2[2]; // shranjene tipke





void LCD_sendC_2(void)
{
    unsigned int i,p;
    
    //LE573hold();    // P4 za LCD in tipke
    LE573hold_2();
    P3DIR |= 0x08;
    LCD_cmd_2(0x0040);  // (2)  Disply start line
   // lezeca postavitev LCD
    p=0xb0;  //Page 0 + konanda 0xb0

    for (i=0;i<1024;i++)
    {
        if ((i&0x7f)==0)
        {
            LCD_cmd_2(0x00); LCD_cmd_2(0x10); // (4) Column Address = 0
            // lezeca postavitev LCD
            LCD_cmd_2(p++); // (3) Page

            //P5OUT|=0x0a;    //RS=1, rd=1
            //P5OUT&=~0x04;       // wr=0
            _set_RS;
            _set_LCD_E2;
            _clear_RW;
            
        }
        
        //P4OUT=LCD[i];
        writeP1P2_out(LCD[i]);
        //P5OUT&=~0x10;   // >CS=0
        //P5OUT|=0x10;    // >CS=1
        _clear_CS;
        _set_CS;
        
    }
    
    //LE573set();   // P4 na 574 izhode
    LE573set_2();
}

void LCD_init_2(void)
{
    int i;
    
    LCD_output_port_2();
    
    //P5OUT&=~0x01;    // LCDreset = 0
    _clear_RES;
    LE573hold_2();    // P4 za LCD in tipke
    //P5OUT|=0x80;     // CSflash=1
    _set_CSFLASH;  // CSflash=1
    KeyBuf_2[0]=KeyBuf_2[1]=0; // shranjene tipke
    //P5OUT|=0x1f;    // wr=1, >CS=1 , E=1, RS=1 , LCDreset = 1
    _set_RW;
    _set_CS;
    _set_LCD_E2;
    _set_RS;
    _set_RES;
    
    i=LCD_read_2(0);
    if (i&0x80)  
        i=LCD_read_2(0);
    LCD_cmd_2(0xa2);  // (11) LCD bias 1/9
    LCD_cmd_2(0xa0);  //(8)  ADC selection normal
    LCD_cmd_2(0xc0);  //(15) Common output normal
    LCD_cmd_2(0x26);  //(17) V5 (Rb/Ra)=6
    LCD_cmd_2(0x81); LCD_cmd_2(0x20-6);  //(18) SET Kontrast 50% Powertip
    LCD_cmd_2(0x2F);  //(21) Power Saver OFF
    LCD_cmd_2(0xaF);  //(1) LCD ON
    LCD_cmd_2(0xa6);  //(9) LCD neinverzni
    LCD_cmd_2(0x0040);  // (2)  Disply start line
    LCD_cmd_2(0x00A4);  // (10) Disply all pos16s OFF (normal)
    LCD_cmd_2(0x00AC);  // (19)  Static indicator OFF
    LCD_cmd_2(0x00F0);  // (22) Test (konec testa)
    LE573set_2();   // P4 na 574 izhode
}

void LCD_input_port_2(void)
{
// vse na input
   P1DIR &= ~(0x80+0x40+         0x08+0x04+0x02+0x01);  
   P2DIR &= ~(0x80+0x40+    0x10+0x08+0x04+0x02+0x01);  
   P3DIR &= ~(                   0x08+0x04+     0x01);   
}
void LCD_output_port_2(void)
{
// vse na input
   P1DIR |= (0x80+0x40+         0x08+0x04+0x02+0x01);  
   P2DIR |= (0x80+0x40+    0x10+0x08+0x04+0x02+0x01);  
   P3DIR |= (                   0x08+0x04          );   
}

void LE573set_2(void)   // P4 na 574 izhode
{
    
    //P4OUT=P4LATCH;  // stanje za LATCH 74HC573
    writeP1P2_out(P4LATCH2);
    //P4SEL = BEEP;
    //P5OUT|= 0x40;    // LE=1
    _set_LE;
    //P5DIR&=~4;      // SOMI vhod
    P3DIR &= ~0x08;
}
void LCD_cmd_2(unsigned char d)
{
    /*
    P5OUT&=~0x0c;    //RS=0, wr=0 (RW=0)
    P5OUT|=0x02;    //rd=1 (E=1)
    writeP1P2_out(d)
    P5OUT&=~0x10;   // >CS=0
    P5OUT|=0x10;    // >CS=1
    //      P7OUT|=0x41;    // wr=1, >CS=1
    */
    _clear_RS;
    __delay_cycles(200);
    _clear_RW;
    __delay_cycles(200);
    _set_LCD_E2;
    __delay_cycles(200);
    writeP1P2_out(d);
    __delay_cycles(200);
    _clear_CS;
    __delay_cycles(200);
    _set_CS;
    __delay_cycles(200);
    
}
void LE573hold_2(void)  // P4 za LCD in tipke
{
    //P5OUT&=~0x40;    // LE=0
    P2OUT&=~0x40;    // LE=0
    __delay_cycles(200);
    //P4LATCH2=P4OUT;  // stanje za LATCH 74HC573
    P4LATCH2  = P2P1_out();
    __delay_cycles(200);
    //P4SEL = 0;
    //P5DIR|= 4;   // SOMI -> LCD_E
    P2DIR |= 0x04;
}
void writeP1P2_out(char data){
    P1OUT = 0;
    P2OUT &= ~0x03;
    if(data & 0x01)P1OUT |= 0x01;
    if(data & 0x02)P2OUT |= 0x02;
    if(data & 0x04)P1OUT |= 0x02;
    if(data & 0x08)P2OUT |= 0x01;
    if(data & 0x10)P1OUT |= 0x04;
    if(data & 0x20)P1OUT |= 0x80;
    if(data & 0x40)P1OUT |= 0x08;
    if(data & 0x80)P1OUT |= 0x40;
}
/*
void writeP1P2_in(char data){
P1OUT = 0;
P2OUT &= ~0x03;
if(data & 0x01) P1IN |= 0x01;
if(data & 0x02) P2IN |= 0x02;
if(data & 0x04) P1IN |= 0x02;
if(data & 0x08) P2IN |= 0x01;
if(data & 0x10) P1IN |= 0x04;
if(data & 0x20) P1IN |= 0x80;
if(data & 0x40) P1IN |= 0x08;
if(data & 0x80) P1IN |= 0x10;

}
*/
char P2P1_out(void){
    char data = 0;
    if(P1OUT & 0x01)data |= 0x01;
    if(P1OUT & 0x02)data |= 0x04;
    if(P1OUT & 0x04)data |= 0x10;
    if(P1OUT & 0x08)data |= 0x40;
    if(P1OUT & 0x20)data |= 0x80;
    //if(P1OUT & 0x020)data |= 0x01;
    //if(P1OUT & 0x040)data |= 0x01;
    if(P1OUT & 0x80)data |= 0x20;  //if(P1OUT & 0x80)data |= 0x40;
    
    if(P2OUT & 0x01)data |= 0x08;
    if(P2OUT & 0x02)data |= 0x02;
    
    return data;
    
}

char P2P1_in(void){
    char data = 0;
    if(P1IN & 0x01)data |= 0x01;
    if(P1IN & 0x02)data |= 0x04;
    if(P1IN & 0x04)data |= 0x10;
    if(P1IN & 0x08)data |= 0x40;
    if(P1IN & 0x20)data |= 0x80;
    //if(P1OUT & 0x020)data |= 0x01;
    //if(P1OUT & 0x040)data |= 0x01;
    if(P1IN & 0x80)data |= 0x20; //if(P1IN & 0x80)data |= 0x40;
    
    if(P2IN & 0x01)data |= 0x08;
    if(P2IN & 0x02)data |= 0x02;
    
    return data;
    
}

char LCD_read_2(int rs)
{
    char d;
    if (rs)
        _set_RS;
    //P5OUT|=0x08;    //RS=1
    else
      _clear_RS;
    __delay_cycles(200);
    
    _set_RW;
    __delay_cycles(200);
    P1P2_input;
    __delay_cycles(200);
    _clear_LCD_E2;
    __delay_cycles(200);
    _clear_CS;
    __delay_cycles(200);
    d = P2P1_in();
    __delay_cycles(200);
    _set_CS;
    __delay_cycles(200);
    P1P2_output;
    __delay_cycles(200);
    return d;
}

void BeriKey_2(void)
{
    unsigned char t,i,tmp;
    
    //Pritisnjene tipke generirajo logièno 0
        
    LE573hold_2();    // P4 za LCD in tipke
    //P4OUT=0;  // vse data linije na 0
    P2OUT &= ~0x03;  P1OUT &= ~0xCF;
    t=0;
    if (TipkaVhod_2()==0)//kadar je pin P3.0 TIPKA na 0, je pritisnjena ena od tipk 
    {
        i=0x20;  //zaène pri Data 5 in gre do Data 1
        //prebere vhod
        while (i)
        {
            tmp = P2P1_out();
            tmp &=~i; 
            writeP1P2_out(tmp);

            if (TipkaVhod_2()==0){
                t|=i;
            }
            i>>=1;      
        }
  	//preveri zakasnitev
        if (DelayKey_2){
            DelayKey_2--;
        }
        else{ // zakasnitev potekla
            if (t & TkRept){
    		//LastKey_2=0; 	   // sprozi ponovitev tipke
                KeyBuf_2[0] = t;   // vpise vrednost
            }
            DelayKey_2=5;   // zakasnitev ponavljanja po novi tipki
        }
    }
    // niè ni pritisnjeno
    else {
        DelayKey_2=5;   // zakasnitev ponavljanja po novi tipki
    }    
    
  
    
    LE573set_2();   // P4 na 574 izhode
  
}

//takrat ko je pin P3.0 TIPKA na 0, je pritisnjena ena od tipk
int TipkaVhod_2(void)
{
    int v;
    v = P3IN;  
    v |= P3IN;
    v &= P3IN;
    
    //P4OUT=0xff;  // vse data linije na 1
    writeP1P2_out(0xFF);
    //P5DIR |=0x20; //P5.5 izhod
    P3DIR |=0x01;
    //P5OUT |=0x20; //P5.5
    P3OUT |=0x01;
    v &=0x01; //P3.0
    //P5DIR &=~0x20; //P5.5 vhod
    P3DIR &=~0x01; //P3.0 vhod
    return v;
}

unsigned char KGet_2(void)
{
    unsigned char t;
    
    if ((t=KeyBuf_2[0])!=0)
    {
        KeyBuf_2[0]= KeyBuf_2[1];
	KeyBuf_2[1]=0;
    }
    return t;
}

void led_diode(void){
    
    if(LCD_vstavljen){
        if(test_koncan){
            if(napaka){
                P6DIR |= (0x08 | 0x10);
                P6OUT |=  0x08; //prižgi rdeèo
                P6OUT &= ~0x10; //ugasne zeleno
                return;
            }
            else{
                P6DIR |= (0x08 | 0x10);
                P6OUT |=  0x10; //prižgi zeleno
                P6OUT &= ~0x08; //ugasne rdeèo
                return;
            } 
        }
        
    }
    P6OUT &= ~0x10; //ugasne zeleno
    P6OUT &= ~0x08; //ugasne rdeèo
}