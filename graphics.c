#include  "msp430x24x.h"
#include  <stdio.h>
#include  "graphics.h"


#define SizRcode 0x500
__no_init unsigned int Rcode[SizRcode/2];   // prostor v RAM za FLASHUP kodo


extern const unsigned int FNflup[];
extern unsigned int Wmmc; 	// oddan znak na MMC
extern const long DekL[10];
int FlashUP(int port);
int MMC_FlashUP(int opt);

void ShraniEE(void);
//******************************************************************************

//******************************************************************************

unsigned long sekunde;   // stevec sekund
unsigned int teden=1565;   // stevec tednov

//extern unsigned char Pisk; // 0 ni piska
unsigned int BeepRol;  // 16-bitni vzorec za pisk
unsigned char OutDev;  // izhodna enota 0=LCD, 1=srednji fonti, 2=veliki fonti, 4=RS232 PC
//unsigned char LCDpr;  //se ne uporablja // stevilka prikaza za LCD

//#pragma data_alignment=2
//__no_init unsigned char LCD[1024];   // vsebina LCD prikazovalnika
//extern unsigned int Rcode[];
//#define LCD ((unsigned char *)&Rcode[0])

//******************************************************************************
extern unsigned int WDdis;   // 1 za disable WD
// ----
#pragma data_alignment=2
unsigned char PosSRB[4]; //kazalci v buferjih za sprejem
#pragma data_alignment=2
unsigned char PosSTB[4]; //kazalci v buferjih za oddajanje
#define  Pos1SRBw ((unsigned char *)&PosSRB)[0]
#define  Pos1SRBr ((unsigned char *)&PosSRB)[1]
#define  Pos1STBw ((unsigned char *)&PosSTB)[0]
#define  Pos1STBr ((unsigned char *)&PosSTB)[1]
#define  Pos2SRBw ((unsigned char *)&PosSRB)[2]
#define  Pos2SRBr ((unsigned char *)&PosSRB)[3]
#define  Pos2STBw ((unsigned char *)&PosSTB)[2]
#define  Pos2STBr ((unsigned char *)&PosSTB)[3]
// za komunikacijo s PC
#define Len1SRB 256
#define Len1STB 256
#pragma data_alignment=2
__no_init unsigned char Ser1RecBuf[Len1SRB];  // sprejemni buffer
#pragma data_alignment=2
//__no_init unsigned char Ser1OutBuf[Len1STB];  // oddajni buffer
#define Ser1OutBuf ((unsigned char *)&Rcode[0x200])
//unsigned char Pos1SRBr,Pos1SRBw,Pos1STBr,Pos1STBw; //kazalci v buferjih za PC
unsigned int RecTim1;   // cas sprejetega znaka
// ----
unsigned int SerRecTim;
unsigned char MBadr;  // MODBUS naslov
unsigned char PSerRecTim;
// ----
// za lokalno komunikacijo
//#define Len2SRB 256
//#define Len2STB 256
#pragma data_alignment=2
__no_init unsigned char Ser2RecBuf[Len2SRB];  // sprejemni buffer
#pragma data_alignment=2
__no_init unsigned char Ser2OutBuf[Len2STB];  // oddajni buffer
//unsigned char Pos2SRBr,Pos2SRBw,Pos2STBr,Pos2STBw; //kazalci v buferjih za PC
unsigned int RecTim2;   // cas sprejetega znaka
// ----
// podatki o serijskih bufferjih
unsigned char *SRecBuf[2]={Ser1RecBuf,Ser2RecBuf};
unsigned char *SOutBuf[2]={Ser1OutBuf,Ser2OutBuf};
const unsigned int LSRecB[2]={Len1SRB-1,Len2SRB-1};
const unsigned int LSOutB[2]={Len1STB-1,Len2STB-1};

//******************************************************************************
int GrX,GrY;   // koordinati tocke na LCD
unsigned int inv;        // za inverzen izpis na LCD
unsigned char DelayKey;  // zakasnitev ponavljanja tipk
unsigned char LastKey;   // zadnja kombinacija tipk
unsigned char KeyBuf[2]; // shranjene tipke
u16 SekStev;   // stevec sekund
s16 PosInv; 		// pozicija zacetka inverznega izpisa
// - - - - -
// za MMC
unsigned int MMCerr;   // stevec napak 
unsigned char MMCstanje;  // 0=ni MMC
unsigned char FATstat; 	  // status FAT16 sistema
long BufSect; 	// stevilka sektorja v bufferju
__no_init unsigned int BufT[256];   // za prenos blokov podatkov
unsigned int FATboot;     // samo 1. particija!
unsigned int FATfat1, FATfat2, FATdir, FATdat;  // samo 1. particija!
unsigned long FATtotSect; //, FATvolID; //, FATtotSectM;
unsigned char FATsnc;
unsigned int CLdirT; 	// zacetni claster trenutnega direktorija
unsigned int CLdirTrk; 	// zacetni claster Track direktorija
unsigned int CLdirMap; 	// zacetni claster Map direktorija
#define ClBufSiz 64
unsigned int Fclast[ClBufSiz];  // zaporedje clastrov datoteke
unsigned int Bclast; 	// zadnji claster datoteke v bufferju
//unsigned int Nclast; 	// stevilo vseh clastrov datoteke
unsigned int Zclast; 	// zacetni claster datoteke
unsigned long Fsize; 	// dolzina datoteke
unsigned long FrdPos; 	// pozicija za branje iz datoteke
unsigned int NFreeCl; 	// stevilo prostih clastrov (ali ffff, ce ni prebrano)
unsigned int FATfreL;   // ali zadnji prost claster
#define FATBufSiz 32
__no_init unsigned int FATfreCL[FATBufSiz]; 	// vreiga prostih clastrov

unsigned int FLupCL; 	// zacetni claster flashup datoteke
unsigned long FLupSiz; 	// dolzina flashup datoteke
unsigned int CLdirFL; 	// zacetni claster Flashup direktorija
unsigned int Nclast, Bclast; 	// stevilo vseh clastrov datoteke, zadnji v bufferju
char test_tipk_biti;
char key_pressed;
//===========================================================================
void SendRS1(unsigned char c);

//===========================================================================

unsigned char P4LATCH;  // stanje za LATCH 74HC573
//===========================================================================
// 74HC573 LACH biti
#define BL 0x01
#define CSSD 0x02
#define BEEP 0x40
#define LED1 0x80
#define LED2 0x20
#define LED3 0x08
//===========================================================================
void LE573set(void)   // P4 na 574 izhode
{
    //  P4DIR = 0xff;   // P4 izhodi
    P4OUT=P4LATCH;  // stanje za LATCH 74HC573
    //P4SEL = BEEP;
    P5OUT|= 0x40;    // LE=1
    P5DIR&=~4;      // SOMI vhod
}
void LE573hold(void)  // P4 za LCD in tipke
{
    P5OUT&=~0x40;    // LE=0
    P4LATCH=P4OUT;  // stanje za LATCH 74HC573
    P4SEL = 0;
    P5DIR|= 4;   // SOMI -> LCD_E
}
//===========================================================================
// porti za SPI data FLASH 
// SPIclk  = P5.3
// SPIsomi = P5.2
// SPIsimo = P5.1
// SPIcs   = P5.7
#define SPI_Clock_1  P5OUT |= 0x08
#define SPI_Clock_0  P5OUT &=~0x08
#define SPI_Data_r  (P5IN & 0x04)
#define SPI_Data_1  P5OUT |= 0x02
#define SPI_Data_0  P5OUT &=~0x02
#define FLASH_CS_1  P5OUT |= 0x80
#define FLASH_CS_0  P5OUT &=~0x80

/* SPI za data FLASH */
/* --------------------------------
serijski DATA FLASH
-------------------------------- */
// stevilo blokov
//#define DFpages  4096
#define DFpages  8192
// velikost bloka (bytov)
#define DFblokS  1024

void ZAKF(void)
{
    asm( " nop ");
    asm( " nop ");
    asm( " nop ");
}
//===========================================================================

void DFSOUT8(unsigned int d)
{
    int i;
    for (i=0x80;i>0;i>>=1)
    {
        if (i&d)
            SPI_Data_1;
        else
            SPI_Data_0;
        SPI_Clock_1;
        SPI_Clock_0;
    }
}
void DFSOUTW(unsigned int d) //LO,HI
{
    DFSOUT8(d);   // LO
    DFSOUT8(d>>8);   // HI
}
void DFSOUT(unsigned int d) //HI,LO
{
    unsigned int i;
    for (i=0x8000;i>0;i>>=1)
    {
        if (i&d)
            SPI_Data_1;
        else
            SPI_Data_0;
        SPI_Clock_1;
        SPI_Clock_0;
    }
}

unsigned int DFSIN8(void);
unsigned int DFSINW(void); //LO,HI
unsigned int DFSIN(void); //HI,LO
/*
unsigned int DFSIN8(void)
{
unsigned int i,d;
d=0;
for (i=0x80;i>0;i>>=1)
{
SPI_Clock_1;
SPI_Clock_0;
if (SPI_Data_r)
d|=i;
    }
return d;
}
unsigned int DFSIN(void)
{
unsigned int i,d;
d=0;
for (i=0x8000;i>0;i>>=1)
{
SPI_Clock_1;
SPI_Clock_0;
if (SPI_Data_r)
d|=i;
    }
return d;
}
*/

int DataFLASH_status(void)
{
    int i;
    SPI_Clock_0;
    FLASH_CS_0;
    DFSOUT8(0xD7);  // opcode D7h
    i=DFSIN8();
    FLASH_CS_1;
    return i;
}

//  CAKANJE NA ZAKLJUCEK UKAZA V DATA FLASH-U
unsigned int FLWAIT(void)
{
    unsigned int i=0;
    while ((DataFLASH_status() & 0x80)==0) i++; //branje STATUS lokacije iz DATA FLASH-a
    return i;
}

void DFUKAZ(int a, int PA, int BA, int n)
{
    SPI_Clock_0;
    FLASH_CS_0;
    DFSOUT8(a);  // opcode 
    //       DFSOUT((PA<<2)|(BA>>8));  // 4M
    DFSOUT((PA<<3)|(BA>>8));    // 8M
    DFSOUT8(BA&0xff);
    if (n)
    {
        while (n)
        {
            DFSOUT8(0xff);
            n--;
        }    
    }
}

void DFcmdPA(int a, int PA)
{
    DFUKAZ(a,PA,0,0);
    FLASH_CS_1;
}


// branje podatkov iz DATA FLASH-a
void DFread(int PA, int BA, int n, unsigned int *BUF)
// PA: -1 ZA BUFFER 2, -2 ZA BUFFER 1, >=0 za FLASH
// BA: odmik od zacetka bloka
// N: stevilo wordov za prenos
// BUF[]: za shranitev podatkov
{
    int i;
    unsigned int a;
    unsigned char *BUFC;
    
    i=1;
    switch (PA)
    {
    case -1: a=0xD6; break;  	// -1 ZA BUFFER 2, -2 ZA BUFFER 1
    case -2: a=0xD4; break;  	//PA: -1 ZA BUFFER 2, -2 ZA BUFFER 1
    default: a=0xD2; i=4; 	// MAIN MEMORY PAGE READ
    }
    DFUKAZ(a,PA,BA,i);
    //	for (i=0;i<n;i++)
    //		BUF[i]=DFSINW();
    P5SEL|=0x0c;
    UCB1TXBUF=0xff;   // sprozi sprejem znaka
    BUFC=(unsigned char *)&BUF[0];
    i=(n<<1)-1;
    do {
        ZAKF();
        a=UCB1RXBUF;  // vsaj 8 taktov od vpisa UCB1TXBUF
        UCB1TXBUF=0xff;   // sprozi sprejem znaka
        *BUFC++=a;
    } while (--i);
    ZAKF();
    *BUFC=UCB1RXBUF;  // vsaj 8 taktov od vpisa UCB1TXBUF
    P5SEL&=~0x0c; 
    FLASH_CS_1;
}


//; VPIS BLOKA N WORDOV IZ BUF[] V DATA FLASH BUFFER b
void DFBwrite(int b, int BA, int n, unsigned int *BUF)
// b: 1 ZA BUFFER 1, 2 ZA BUFFER 2
// BA: odmik od zacetka bloka
// N: stevilo wordov za prenos
// BUF[]: podatki
{
    int i;
    unsigned int a;
    unsigned char *BUFC;
    
    if (b==2)
        a=0x87;  // BUFFER 2 WRITE
    else
        a=0x84;  // BUFFER 1 WRITE
    DFUKAZ(a,0,BA,0);
    //	for (i=0;i<n;i++)
    //		DFSOUTW(BUF[i]);
    UCB1CTL0 = UCSYNC+UCMST+UCMSB+UCCKPH;
    P5SEL|=0x0e;
    BUFC=(unsigned char *)&BUF[0];
    i=(n<<1);
    do {
        UCB1TXBUF=*BUFC;  // vsaj 8 taktov za korak zanke
        ZAKF();
        *BUFC++;
    } while (--i);
    P5SEL&=~0x0e;
    UCB1CTL0 = UCSYNC+UCMST+UCMSB;
    FLASH_CS_1;
}

// PRENOS STRANI PA iz DATA FLASH v BUFFER 1
void DFB1trans(int PA)
{
    DFcmdPA(0x53,PA);  // PAGE TO BUFFER 1 TRANSFER
    FLWAIT(); 		// cakanje na izvrsitev
}

// PRENOS STRANI PA iz DATA FLASH v BUFFER 2
void DFB2trans(int PA)
{
    DFcmdPA(0x55,PA);  // PAGE TO BUFFER 2 TRANSFER
    FLWAIT(); 		// cakanje na izvrsitev
}

//  BRISANJE STRANI PA
void DFPerase(int PA)
{
    DFcmdPA(0x81,PA); // ;PAGE ERASE
}

// PROGRAMIRANJE STRANI PA (RW7) V DATA FLASH IZ BUFFERJA
void DFPwriteB1(int PA)
{
    DFcmdPA(0x88,PA); //  ;BUFFER 1 TO PAGE PROGRAM (BREZ BRISANJA)
}
void DFPwriteB2(int PA)
{
    DFcmdPA(0x89,PA); //  ;BUFFER 2 TO PAGE PROGRAM (BREZ BRISANJA)
}
void DFPEraseWriteB1(int PA)
{
    DFcmdPA(0x83,PA); //  ;BUFFER 1 TO PAGE PROGRAM (Z BRISANJEM)
}
void DFPEraseWriteB2(int PA)
{
    DFcmdPA(0x86,PA); //  ;BUFFER 2 TO PAGE PROGRAM (Z BRISANJEM)
}

//  TESTIRANJE PRAZNEGA BLOKA (PA) OD NASLOVA (BA) NAPREJ
int TeFLDBlk(int PA, int BA)
// BA: odmik od zacetka bloka
{
    int i,n;
    unsigned char a;
    
    /*	
    n=(DFblokS-BA)>>1;
    DFUKAZ(0xD2,PA,BA,4); 	// MAIN MEMORY PAGE READ
    for (i=0;i<n;i++)
    if (DFSINW()!=0xffff) // potrebno brisanje
    {
    FLASH_CS_1;
    DFPerase(PA); 	//  BRISANJE STRANI PA
    FLWAIT(); 		// cakanje na izvrsitev
    return 1;   	// zakjucek z brisanjem
}
    */
    DFUKAZ(0xD2,PA,BA,4); 	// MAIN MEMORY PAGE READ
    P5SEL|=0x0c;
    UCB1TXBUF=0xff;   // sprozi sprejem znaka
    n=(DFblokS-BA);
    for (i=1;i<n;i++)
    {
        ZAKF();
        a=UCB1RXBUF;  // vsaj 8 taktov od vpisa UCB1TXBUF
        UCB1TXBUF=0xff;   // sprozi sprejem znaka
        if (a!=0xff) // potrebno brisanje
            i=n;      // konec zanke
    }
    ZAKF();
    P5SEL&=~0x0c; 
    FLASH_CS_1;
    a&=UCB1RXBUF;  // vsaj 8 taktov od vpisa UCB1TXBUF
    if (a!=0xff) // potrebno brisanje
    {
        DFPerase(PA); 	//  BRISANJE STRANI PA
        FLWAIT(); 		// cakanje na izvrsitev
        return 1;   	// zakjucek z brisanjem
    }
    return 0; 	// ni bilo brisanja
}

//// branje podatkov iz DATA FLASH-a direktno na RS za PC
//void DFreadRS(int PA, int BA, int n)
//// BA: odmik od zacetka bloka
//// N: stevilo wordov za prenos
//{
//  int i;
////  unsigned int ah,al,t;
//  unsigned int a,t;
//
//  _DINT();          // Enable interrupts
//  TBCTL=TBSSEL_1+MC_2;	//ACLK, Cont up
//  t=TBR;   // stevec prekinitev 32768/2 Hz
//	DFUKAZ(0xD2,PA,BA,4);// MAIN MEMORY PAGE READ
///*
//	ah=DFSIN8();
//	for (i=1;i<n;i++)
//	{
//          DPUT1(ah);  //direkten vpis znaka na RS ua PC
//    	  al=DFSIN8();
//          DPUT1(al);  //direkten vpis znaka na RS ua PC
//    	  al=DFSIN8();
////          DPUT1(ah);  //direkten vpis znaka na RS ua PC
//    	  ah=al;
///-*	  a=DFSIN();
//          put1RS(a&0xff);  //lo
//          put1RS(a>>8);    //hi *-/
//	}
//        DPUT1(ah);  //direkten vpis znaka na RS ua PC
//        al=DFSIN8();
//        DPUT1(al);  //direkten vpis znaka na RS ua PC
//*/
//	P5SEL|=0x0c;
//	UCB1TXBUF=0xff;   // sprozi sprejem znaka
//        i=(n<<1)-1;
//	P5SEL|=0x04;      // dodatna zakasnitev
//	do {
//		a=UCB1RXBUF;  // vsaj 8 taktov od vpisa UCB1TXBUF
//		UCB1TXBUF=0xff;   // sprozi sprejem znaka
//                DPUT1(a);  //direkten vpis znaka na RS ua PC
//	} while (--i);
//        DPUT1(UCB1RXBUF);  // vsaj 8 taktov od vpisa UCB1TXBUF
//	P5SEL&=~0x0c; 
//	FLASH_CS_1;
////        DPUT1(ah);  //direkten vpis znaka na RS ua PC
////  TBCTL=0;  // stop timer B
//  t=(TBR-t)>>1;   // stevec prekinitev 32768/2 Hz
//  i=StevACLK&0x3fff;  // stevec prekinitev 32768/2 Hz  
//  StevACLK+=t;   // stevec prekinitev 32768/2 Hz
//  i=(i+t)>>14;   // prenos sekund
//  sekunde+=i;
///*  Oldcapture = CCR2-DELTA;   // Save current captured SMCLK
//  StevACLK|=3;       // stevec prekinitev 32768/2 Hz  
//  _EINT();          // Enable interrupts */
//}

//===========================================================================
// MMC kartica

unsigned int addCRC(unsigned int crc,unsigned char value)
{
    //   crc  = (int)(crc >> 8) | (crc << 8);
    asm(" SWPB R12");
    crc ^= value;
    crc ^= (int)(crc & 0xff) >> 4;
    crc ^= (crc << 8) << 4;
    crc ^= ((crc & 0xff) << 4) << 1;
    return crc;
}
#if 1
unsigned char R_CRC7(unsigned char * chr, int cnt); // ASM rutina za RAM
#define CRC7 R_CRC7
#else
unsigned char CRC7(unsigned char * chr, int cnt)
{
    int i; //,a;
    unsigned char crc,Data; //,d1;
    
    crc=0;
    //  for (a=0;a<cnt;a++)
    do {
        //   crc=addCRC(crc,chr[a]);
        //	Data=chr[a];
	Data=*chr++;
	i=8;
        //	for (i=0;i<8;i++)
        do {
            crc <<= 1;
            //		if ((Data & 0x80)^(crc & 0x80))
            if ((signed char)(Data ^ crc)<0)
                crc ^=0x09; 
            Data <<= 1;
	} while (--i);
    } while (--cnt);
    crc=(crc<<1)|1;
    return crc;
}
#endif

// porti za SPI MMC
// MMCclk  = P5.3
// MMCsomi = P5.2
// MMCsimo = P5.1
// MMCcs   = P4.1
#define MMC_Clock_1  P5OUT |= 0x08
#define MMC_Clock_0  P5OUT &=~0x08
#define MMC_Data_r  (P5IN & 0x04)
#define MMC_Data_1  P5OUT |= 0x02
#define MMC_Data_0  P5OUT &=~0x02
//  P3DIR = 0x0b;                             // P3 izhodi
#define MMC_CS_1  P4OUT |= CSSD
#define MMC_CS_0  P4OUT &=~CSSD
#define MMC_CS_1a  P4OUT |= CSSD
#define MMC_CS_0a  P4OUT &=~CSSD


#if 1
void R_mcb_xmit(unsigned int d); // ASM rutina za RAM
#define mcb_xmit R_mcb_xmit
#else
void mcb_xmit(unsigned int d)
{
    UCB1CTL0 = UCSYNC+UCMST+UCMSB+UCCKPH;
    UCB1TXBUF=d;
    asm ("  CMP.W @SP,R12     ;  zakasnitev 2 takta  ");
    asm ("  CMP.W @SP,R12     ;  zakasnitev 2 takta  ");
}
/*
int i;
i=0x80;
//  for (i=0x80;i>0;i>>=1)
do {
if (i&d)
MMC_Data_1;
    else
MMC_Data_0;
MMC_Clock_1;
MMC_Clock_0;
i>>=1;
  } while (i); 
} */
#endif
/*
void mcb_xmitX(unsigned int d)
//void MMCSOUT8(unsigned int d)
{
int i;
for (i=0x80;i>0;i>>=1)
{
if (i&d)
MMC_Data_1;
    else
MMC_Data_0;
MMC_Clock_1;
MMC_Clock_0;
    }
}
*/
#if 1
unsigned int R_mcb_rec(void); // ASM rutina za RAM
#define mcb_rec R_mcb_rec
#else
unsigned int mcb_rec(void)
{
    UCB1TXBUF=0xff;   // sprozi sprejem znaka
    asm ("  MOV.B @SP,R12     ;  zakasnitev 2 takta  ");
    asm ("  MOV.B @SP,R12     ;  zakasnitev 2 takta  ");
    asm ("  MOV.B @SP,R12     ;  zakasnitev 2 takta  ");
    asm ("  MOV.B @SP,R12     ;  zakasnitev 2 takta  ");
    asm ("  MOV.B @SP,R12     ;  zakasnitev 2 takta  ");
    asm ("  MOV.B @SP,R12     ;  zakasnitev 2 takta  ");
    asm ("  MOV.B @SP,R12     ;  zakasnitev 2 takta  ");
    asm ("  MOV.B @SP,R12     ;  zakasnitev 2 takta  ");
    return UCB1RXBUF;  // vsaj 8 taktov od vpisa UCB1TXBUF
}
#endif
/*
unsigned int MMCSIN8(void);
unsigned int MMCSIN(void);
{
unsigned int i,d;
MMC_Data_1;
d=0;
for (i=0x80;i>0;i>>=1)
{
MMC_Clock_1;
if (MMC_Data_r)
d|=i;
MMC_Clock_0;
    }
return d;
}
*/

unsigned int MMC_wait(int max)
{
    unsigned int i,j;
    UCB1CTL0 = UCSYNC+UCMST+UCMSB;
    j=0;
    do {
        //		McbspaRegs.DXR1.all=0xffff;   // za odgovor
        i=mcb_rec();
        j++;
    } while ((i==0xff)&(j<max));
    if (j==max)
        return 0xffff;
    return i;
}

unsigned int MMC_cmd(int cmd, unsigned long par)
{
    unsigned char buf[6];
    unsigned char *bp;
    unsigned int i;
    
    P4OUT |= LED1;  // vklop MMC LED
    buf[0]=cmd|0x40;
    buf[2]=par>>16;
    buf[1]=par>>24;
    buf[4]=par;
    buf[3]=par>>8;
    buf[5]=CRC7(buf,5);
    P5SEL|=0x0e;
    MMC_CS_0;
    bp=&buf[0];
    i=6;
    //	for (i=0;i<6;i++)
    do {
        //		mcb_xmit(buf[i]);
        mcb_xmit(*bp++);
        //		mcb_rec();
    } while (--i);
    //	McbspaRegs.DXR1.all=buf[5];
    /*	for (i=0;i<6;i+=2)
    McbspaRegs.DXR1.all=(buf[i]<<8)+(buf[i+1]&0xff);
    for (i=0;i<3;i++)
    mcb_rec();		
    for (i=0;i<6;i++)   
    McbspaRegs.DXR1.all=buf[i];	 */
    /*	for (i=0;i<6;i++)
    mcb_rec();		*/
    return MMC_wait(9);
}

unsigned int MMCstatus(void)
{
    unsigned int i;
    i=MMC_cmd(13, 0);  //SEND_STATUS
    //	McbspaRegs.DXR1.all=0xffff;   // za odgovor
    i=(i<<8)+mcb_rec();
    MMC_CS_1;
    P5SEL&=~0x0e;
    P4OUT &=~LED1;  // izklop MMC LED
    return i;
}
/*
int MMCrd(int cmd, int len, unsigned int *BUF)
{
int i,j;
i=MMC_cmd(cmd, 0);
j=len-1;
i=MMC_wait(9);
if (i!=0xfe)
{
//		McbspaRegs.DXR1.all=0xffff;   // za odgovor
BUF[j--]=(i<<8)+mcb_rec();	
	}  
for (;j>=0;j--)
{
//		McbspaRegs.DXR1.all=0xffff;   // za odgovor
//		McbspaRegs.DXR1.all=0xffff;   // za odgovor
BUF[j]=(mcb_rec()<<8);
BUF[j]+=mcb_rec();
	}
MMC_CS_1;
P4OUT &=~LED1;  // izklop MMC LED
return i;
}
*/
/*
struct  CID_BITS {          // bits  description
unsigned int    r1:1;       // 0   VEDNO 1
unsigned int    CRC:7;      // 1:7 CRC
unsigned int    MDT:8;      // 8:15  Manufacturing date
unsigned int    PSNlo:16;     // 16:47 Product SN
unsigned int    PSNhi:16;     // 16:47 Product SN
unsigned int    PRV:8;      // 48:55 Product revision
unsigned int    PNM5:8;     // 56:103 Product Name
unsigned int    PNM4:8;     // 56:103 Product Name
unsigned int    PNM3:8;     // 56:103 Product Name
unsigned int    PNM2:8;     // 56:103 Product Name
unsigned int    PNM1:8;     // 56:103 Product Name
unsigned int    PNM0:8;     // 56:103 Product Name 
unsigned int    OIDl:8;     // 104:119 OEM ID
unsigned int    OIDh:8;     // 104:119 OEM ID
unsigned int    MID:8;      // 120:127 Manufacturer ID (==9x06)
} MMC_CID; 
*/

int MMCidle(void)  // poslje GO_IDLE_STATE s CRC
{
    int i;
    i=MMC_cmd(0, 0);  // poslje GO_IDLE_STATE s CRC
    MMC_CS_1;
    P5SEL&=~0x0e;
    return i;
}

int MMCinit(void)
{
    int i;
    char j;
    
    MMC_CS_1a;
    P5SEL|=0x0e;
    //	MMC_CS_1;
    //	MCBinit();
    i=10; // generiranje 80 clk pulzov
    //	for (i=0;i<10;i++)    // generiranje 80 clk pulzov
    do {
        mcb_rec();		
    } while (--i);
    i=MMC_cmd(0, 0);  // poslje GO_IDLE_STATE s CRC
    MMC_CS_1;
    P5SEL&=~0x0e;
    P4OUT &=~LED1;  // izklop MMC LED
    if ((i&0xff)!=1)
    {
        i=MMC_cmd(0, 0);  // poslje GO_IDLE_STATE s CRC
        MMC_CS_1;
        P5SEL&=~0x0e;
        P4OUT &=~LED1;  // izklop MMC LED
        if ((i&0xff)!=1)
            return 0;   // ni kartice
    }
    //	MCBinit();
    j=255;
    do {
        //		ZakT(2000L*Fzak);  // Tcss-50
        i=MMC_cmd(1, 0); // poslje SEND_OP_COND s CRC
        MMC_CS_1;
        P5SEL&=~0x0e;
        if (--j==0)
            return 0;   // napaka kartice
    } while ((i&0xff)==1);
    //	McbspaRegs.SRGR1.all=0x0005;  // LSPCLK/6 =7.5MHz
    //	ZakT(5000L*Fzak);  // 5us
    /*	i=MMC_cmd(1, 0); // poslje SEND_OP_COND s CRC
    MMC_CS_1; */
    i=MMCstatus();	  //SEND_STATUS
    //	MMC_OCR=MMCrdOCR();  //READ_OCR
    //	i=MMCrd(9, 8, (unsigned int *) &MMC_CSD);  //SEND_CSD
    //--	i=MMCrd(10, 8, (unsigned int *) &MMC_CID);  //SEND_CID
    //        P4OUT &=~LED1;  // izklop MMC LED
    
    /*	i=MMC_cmd(0, 0);  // poslje GO_IDLE_STATE s CRC
    MMC_CS_1; */
    // dodatna 2 CLK
    MMC_Clock_1;
    MMC_Clock_0;
    MMC_Clock_1;
    MMC_Clock_0;
    return 1;
}

// branje bloka 512 bytov podatkov iz MMC
int MMCread(unsigned long adr, unsigned char *BUF)
{
    int i,j;
    unsigned int crc;
    unsigned char val;
    
    crc=0;
    i=MMC_cmd(17, adr);  //READ_SINGLE_BLOCK
    j=MMC_wait(1000);
    j=512;
    //      if (Power&0x10)   // GPS
    /*        {
    do {
    val=MMCSIN8();  //mcb_rec();
    *BUF++=val;
    crc=addCRC(crc, val);
} while (--j);
}
	else */
    {
        //	  P5SEL|=0x0c;
        val=0;
        do {
            UCB1TXBUF=0xff;   // sprozi sprejem znaka
            crc=addCRC(crc, val);
            val=UCB1RXBUF;
            *BUF++=val;
        } while (--j);
        //	  P5SEL&=~0x0c; 
        crc=addCRC(crc, val);
    }  
    //	McbspaRegs.DXR1.all=0xffff;   // za odgovor
    j=(mcb_rec()<<8);  //CRC
    j+=mcb_rec();  
    //	j=MMCSIN(); //CRC hi/lo
    MMC_CS_1;
    P5SEL&=~0x0e;
    P4OUT &=~LED1;  // izklop MMC LED
    // dodatna 2 CLK za zmanjsanje porabe
    MMC_Clock_1;
    MMC_Clock_0;
    MMC_Clock_1;
    MMC_Clock_0;
    if (crc!=j)
    {
        MMCerr++;   // stevec napak 
        return 256;   // napacen CRC
    }
    return i;
}

unsigned int MMC_wrwait(int max)
{
    int i,j;
    j=0;
    do {
        P5SEL|=0x0e;
        MMC_CS_0;
        P4OUT |= LED1;  // vklop MMC LED
        //		McbspaRegs.DXR1.all=0xffff;   // prazen znak
        i=mcb_rec();
        MMC_CS_1;
        P5SEL&=~0x0e;
        P4OUT &=~LED1;  // izklop MMC LED
        j++;
    } while ((i!=0xff)&(j<max));
    return j;
}

// zapis bloka 512 bytov podatkov na MMC
int MMCwrite(unsigned long adr, unsigned char *BUF)
{
    int i,j;
    unsigned int crc;
    unsigned char val;
    
    crc=0;
    i=MMC_cmd(24, adr);  //WRITE_BLOCK
    mcb_xmit(0xff);   // 1x Nwr prazen znak
    mcb_xmit(0xfe);   // Start Block Write
    //	mcb_rec();
    //	mcb_rec();
    j=512;
    //	for (j=0;j<512;j++)
    UCB1CTL0 = UCSYNC+UCMST+UCMSB+UCCKPH;
    P5SEL|=0x0e;
    do {
        val=*BUF++;
        UCB1TXBUF=val;
        crc=addCRC(crc, val);
    } while (--j);
    //	P5SEL&=~0x0e;
    //        UCB1CTL0 = UCSYNC+UCMST+UCMSB;
    /*
    do {
    //		val=BUF[j]&0xff;  //lo
    val=*BUF++;
    mcb_xmit(val);
    crc=addCRC(crc, val);
    /-*		val=BUF[j]>>8;  //hi
    mcb_xmit(val);
    crc=addCRC(crc, val); *-/
    //		mcb_rec();
    //		mcb_rec();
} while (--j);
    */
    mcb_xmit(crc>>8);   // CRC hi
    mcb_xmit(crc&0xff); // CRC lo
    //	mcb_rec();
    //	mcb_rec();
    i=MMC_wait(1000);
    mcb_rec();  // dodatnih 8 clockov
    MMC_CS_1;
    P5SEL&=~0x0e;
    //--	i=MMCstatus();	  //SEND_STATUS
    return i;
}
//===========================================================================
// branje sektorja 's' v 'BufT'
int MMCreadSect(long s)
{
    int i,r,ern;
    
    if (BufSect!=s) 	// stevilka sektorja v bufferju
    {
        //                if (ModeF<=4)   // ni PC prenos
        //		  rutine1();    // obdelovalne rutine brez LCD izpisa
        r=4;
        ern=8;
        do {
            i=MMCread(s<<9, (unsigned char *)&BufT);
            if (i==0)
            {
                BufSect=s;  	// stevilka sektorja v bufferju
                return 0; 	//o.k.
            }
            if (--r==0)
                return i; 	// napaka
            MMCinit();
        } while (ern--);
        /*		while ((i!=0)&&(r<4))
        {
        r++;
        MMCinit();
        i=MMCread(s<<9, (unsigned char *)&BufT);
    }
        if (i)
        {
        //  printf(" R %lu %d\r",s,i);
        return i; 	// napaka
    } 
        BufSect=s;  	// stevilka sektorja v bufferju  */		
    }
    return 0; 	//o.k.
}
//=====================
/*
int MMCreadSectX(long s)
{
int i,r;

if (BufSect!=s) 	// stevilka sektorja v bufferju
{
if (ModeF!=5)   // PC prenos
rutine1();    // obdelovalne rutine brez LCD izpisa
r=0;
i=MMCread(s<<9, (unsigned char *)&BufT);
while ((i!=0)&&(r<5))
{
r++;
MMCinit();
i=MMCread(s<<9, (unsigned char *)&BufT);
		}
if (i)
{
//  printf(" R %lu %d\r",s,i);
return i; 	// napaka
		}
BufSect=s;  	// stevilka sektorja v bufferju
	}
return 0; 	//o.k.
}
//=====================
*/

// zapis sektorja 's' iz 'BufT'
int MMCwriteSect(long s, int re)
{
    int i,r,ern;
    
    BufSect=s;  	// stevilka sektorja v bufferju
    r=4;
    ern=8;
    do {
        i=MMCwrite(s<<9, (unsigned char *)&BufT); 	// zapis bloka 512 bytov podatkov na MMC
        i&=0x1f;
        if (i==0x05)  // o.k.
        {
            //	    if (re)
            //              if (ModeF<=4)   // ni PC prenos
            // 		  rutine1();   // obdelovalne rutine brez LCD izpisa
	    i=MMC_wrwait(5000);
	    if (i>=5000)
                i=MMC_wrwait(15000);
            //		pdecw(i);
            //  printf("%d\r",i);
	    return 0; 	//o.k.
        }
        if (--r)
	    return i; 	// napaka
        MMCinit();
    } while (ern--);
    return -1;
}
/*
//  printf(" W %lu %x ",BufSect,i);
while (((i&0x1f)!=0x05)&&(r<4))
{
r++;
MMCinit();
i=MMCwrite(s<<9, (unsigned char *)&BufT); 	// zapis bloka 512 bytov podatkov na MMC
	}
if (r>=4)
{
//  printf("ERR %d %x\r",r,i);
return i; 	// napaka
	}
if (re)
if (ModeF!=5)   // PC prenos
rutine1();   // obdelovalne rutine brez LCD izpisa
i=MMC_wrwait(5000);
if (i>=5000)
{
r=0;
while ((i>=5000)&&(r<4))
{
r++;
i=MMC_wrwait(5000);
		}
	}
//		pdecw(i);
//  printf("%d\r",i);
return 0; 	//o.k.
}
*/
//=====================
/*
int MMCwriteSectX(long s, int re)
{
int i,r;

BufSect=s;  	// stevilka sektorja v bufferju
r=0;
i=MMCwrite(s<<9, (unsigned char *)&BufT); 	// zapis bloka 512 bytov podatkov na MMC
printf(" W %lu %x ",BufSect,i);
while (((i&0x1f)!=0x05)&&(r<4))
{
r++;
MMCinit();
i=MMCwrite(s<<9, (unsigned char *)&BufT); 	// zapis bloka 512 bytov podatkov na MMC
	}
if (r>=4)
{
//  printf("ERR %d %x\r",r,i);
return i; 	// napaka
	}
if (re)
if (ModeF!=5)   // PC prenos
rutine1();   // obdelovalne rutine brez LCD izpisa
i=MMC_wrwait(5000);
if (i>=5000)
{
r=0;
while ((i>=5000)&&(r<5))
{
r++;
i=MMC_wrwait(5000);
		}
	}
//		pdecw(i);
printf("%d\r",i);
return 0; 	//o.k.
} */
//=====================

// nastavitev parametrov FAT16
int MMCrInfo(void)
{
    //	unsigned int i;
    unsigned long fs;
    
    FATstat=1; 	// status FAT16 sistema
    BufSect|=0xffff0000; 	// stevilka sektorja v bufferju !=0
    if (MMCreadSect(0))
        return FATstat; 	// status FAT16 sistema
    //-	FATstat++; 	// status FAT16 sistema
    if ((BufT[0]!=0x3ceb)||((BufT[1]&0xff)!=0x90))	// ni boot record
    {
        if (BufT[0xe4])   // boot sektor hi
            return FATstat; 	// ni podpore za long vrednosti!
        FATboot=BufT[0xe3];
        //		FATboot=((unsigned long)BufT[0xe4]<<16)|BufT[0xe3];
        //		((unsigned int *)&FATboot)[0]=BufT[0xe3];
        //		((unsigned int *)&FATboot)[1]=BufT[0xe4];
        /*
        //		FATtotSectM=((unsigned long)BufT[0xe6]<<16)|BufT[0xe5];
        ((unsigned int *)&FATtotSectM)[0]=BufT[0xe5];
        ((unsigned int *)&FATtotSectM)[1]=BufT[0xe6];
        */
        if ((BufT[0xe1]&0xff)!=6) 	//  16-bit FAT (Partition Larger than 32MB)
            return 	FATstat; 	// status FAT16 sistema
        //-		FATstat++; 	// status FAT16 sistema
        if (MMCreadSect(FATboot))
            return 	FATstat; 	// status FAT16 sistema
    }
    else
    {
        FATboot=0;
        //		FATtotSectM=0;
        //-		FATstat++; 	// status FAT16 sistema
    }
    //-	FATstat++; 	// status FAT16 sistema
    //	FATsnc=BufT[6]>>8;  // Sectors/cluster
    FATsnc=((unsigned char *)&BufT[6])[1];     // Sectors/cluster
    //	FATtotSect=((unsigned long)BufT[17]<<16)|BufT[16];	// Number of total sectors	
    ((unsigned int *)&FATtotSect)[0]=BufT[16];  // Number of total sectors	LO
    ((unsigned int *)&FATtotSect)[1]=BufT[17];  // Number of total sectors	HI
    FATfat1=FATboot+BufT[7];  // zacetni sektor FAT1 
    //	BufT[7] Reserved Sectors (Number of reserved sectors at the beginning 
    //  of the media includingthe boot sector.) typical value = 1
    fs=BufT[11];  //Sectors per FAT  ((FATtotSect/FATsnc)>>8)+1;  //dlozina FAT tabele
    FATfat2=FATfat1+fs;
    FATdir=FATfat2+fs;
    if ((BufT[6]&0xff)!=2) 	// 512 Bytes per Sector
        return 	FATstat; 	// status FAT16 sistema
    FATdat=FATdir+((BufT[9]&0xff)<<4); // HI Number of possible root entries - 512 entries
    if (BufT[8]!=2) 	// Number of FAT copies
        return 	FATstat; 	// status FAT16 sistema
    if (BufT[255]!=0xaa55) 	// Boot sector signature - This is the AA55h signature.
        return 	FATstat; 	// status FAT16 sistema
    return FATstat=0;
}

//===========================================================================
// iskanje datoteke v bloku direktorija
/*
int FindFile(unsigned int *BUF, unsigned int *name)
{
unsigned int i,j;
for (i=0;i<256;i+=16)
{
for (j=0;j<5;j++)
if ((BUF[i+j]|0x2020)!=(name[j]|0x2020)) j=32; 	// velike->male crke
if (j<32) 	//ujemanje prvih 10 znakov
if (((BUF[i+5]&0xff)|0x20)==((name[5]&0xff)|0x20))  //ujemanje vseh 11 znakov
{
j=name[5]>>8; //atributi
if (j)   //zahtevan atribut za direktorij 0x10
{
if ((BUF[i+5]>>8)==j) //atributi
return i;
				}
				else 	// ne sme biti direktorij
{
if ((BUF[i+5]>>8)!=0x10) //atributi
return i;
				}
			}
	}
return -1; 	// ni najdeno
}
*/
int FindFile(unsigned char *BUF, unsigned char *name)
{
    int i,j;
    unsigned char *n, *b;
    i=0;
    //	for (i=0;i<512;i+=32)
    do {
        b=&BUF[i];
        n=name;
        j=11;
        do {
            if ((*b++^*n++)&(~0x20)) // velike->male crke
                j=-1;   // neujemanje
            j--;
        } while (j>0);
        //		for (j=0;j<11;j++)
        //			if ((BUF[i+j]|0x20)!=(name[j]|0x20)) j=32; 	// velike->male crke
        //		if (j<32)   //ujemanje prvih 11 znakov
        if (j==0)   //ujemanje prvih 11 znakov
            //			if (((BUF[i+5]&0xff)|0x20)==((name[5]&0xff)|0x20))  //ujemanje vseh 11 znakov
        {
            //				j=name[11]; //atributi
            j=*n; //atributi
            if (j)   //zahtevan atribut za direktorij 0x10
            {
                //					if ((BUF[i+11])==j) //atributi
                if (*b=j) //atributi
                    return i>>1;
            }
            else 	// ne sme biti direktorij
            {
                //					if ((BUF[i+11])!=0x10) //atributi
                if (*b!=0x10) //atributi
                    return i>>1;
            }
        }
        i+=32;
    } while (i<512);
    return -1; 	// ni najdeno
}

int FnextCl(void);

int LookSubDir(unsigned int cl, unsigned int *name, int NN, unsigned int *ZC, unsigned long *Siz)
{
    // iskanje NN datotek z imeni name[] v poddirektoriju 'cl'
    // rezulati: ZC[] zacetni claster, Siz[] dolzina
    int i,n,f,r;
    unsigned int c;
    unsigned long s;
    
    for (i=0;i<NN;i++)
    {
        ZC[i]=0;
        Siz[i]=0;
    }
    r=0;
    Bclast=0; 		 // zadnji claster v bufferju
    Nclast=ClBufSiz; // stevilo vseh clastrov datoteke (najvecja vrednost)
    Fclast[0]=cl;  	 // zaporedje clastrov datoteke
    FnextCl(); 		// zakjucek z -1 (predcasen konec verige)
    for (n=0;n<=Bclast;n++)
    {
        c=Fclast[n];
        s=FATdat+(unsigned long)(c-2)*FATsnc;
        for (c=0;c<FATsnc;c++)
        {
            if (MMCreadSect(s++))
                return 	r; 	// napaka pri branju
            for (f=0;f<NN;f++)
            {
                i=FindFile((unsigned char *)&BufT, (unsigned char *)&name[f*6]);
                if (i>=0)
                {
                    ZC[f]=BufT[i+13]; 	// zacetni claster datoteke
                    Siz[f]=((unsigned long)BufT[i+15]<<16)+BufT[i+14]; 	// dolzina datoteke
                    r++;
                    if (r==NN)
                        return r; //konec iskanja
                }
            }
        }
    }
    return r;
}

//const unsigned int FNtrack[6]={'T'+256*'R','A'+256*'C','K'+256*' ',' '+256*' ',
//			' '+256*' ',' '+0x1000 };
//const unsigned int FNmap[6]={'M'+256*'A','P'+256*' ',' '+256*' ',' '+256*' ',
//			' '+256*' ',' '+0x1000 };
const unsigned int FNflash[6]={'U'+256*'P','G'+256*'R','A'+256*'D','E'+256*' ',
' '+256*' ',' '+0x1000 };
/*const unsigned int FNdata[6] = {'D'+256*'A','T'+256*'A',' '+256*' ',' '+256*' ',
' '+256*' ',' '+0x1000 };
const unsigned int FNauto[6] = {'A'+256*'U','T'+256*'O','M'+256*'E','N'+256*'U',
'T'+256*'X','T' }; */

// iskanje privzetih datotek in direktorijev
int LookDir(void)
{
    int i,n;
    //	unsigned long s;
    unsigned int s;
    //	unsigned int FNflup[6] = {'R'+256*'E','G'+256*'U','L'+256*' ',' '+256*' ',
    //			'F'+256*'L','2' };
    
    //	CLdirTrk=0; 	// zacetni claster Track direktorija
    //	CLdirMap=0; 	// zacetni claster Map direktorija
    FLupCL=0; 	// zacetni claster flashup datoteke
    CLdirFL=0; 	// zacetni claster Flashup direktorija
    n=0;
    if (MMCrInfo())	// nastavitev parametrov FAT16
        return 0; 	// napaka
    s=FATdir;
    //	for (s=FATdir; s<FATdat; s++)
    do {
        if (MMCreadSect(s))
            return 	0; 	// napaka pri branju
        //		i=FindFile((unsigned char *)&BufT, (unsigned char *)FNtrack);
        //		if (i>=0)
        //			{
        //				CLdirTrk=BufT[i+13]; 	// zacetni claster Track direktorija
        //				n++;
        //			}
        //		i=FindFile((unsigned char *)&BufT, (unsigned char *)FNmap);
        //		if (i>=0)
        //			{
        //				CLdirMap=BufT[i+13]; 	// zacetni claster Map direktorija
        //				n++;
        //			}
        i=FindFile((unsigned char *)&BufT, (unsigned char *)FNflash);
        if (i>=0)
        {
            CLdirFL=BufT[i+13]; 	// zacetni claster Flashup direktorija
            n++;
        }
        /*		i=FindFile((unsigned char *)&BufT, (unsigned char *)FNauto);
        if (i>=0)
        {
        FLauto=1; 	// status prisotnosti AUTOMENU.TXT
        n++;
    } */
        //		if ((n==2)||(BufT[0]==0))
        //			return n;
        if ((n==1)||(BufT[0]==0))
            s=FATdat; 	// vse najdeno
        s++;
    } while (s<FATdat);
    if (CLdirFL) 	// zacetni claster Flashup direktorija
    {
        //		Bcopy(&TModel[0],&FNflup[0],3);	// 6 znakov tipske oznake
        n+=LookSubDir(CLdirFL,(unsigned int *)FNflup,1,&FLupCL,&FLupSiz);
    }
    return n;
}

unsigned int RNclast(unsigned long size)
{
    return (((size-1)>>9)/FATsnc)+1; //stevilo vseh clastrov datoteke
}

// izracun zacetnega sektorja clastra cl
unsigned long RacClastS(unsigned int cl)
{
    return FATdat+(unsigned long)(cl-2)*FATsnc;
}

//unsigned long RacSect(unsigned long pos, unsigned int cl)
unsigned long RacSect(unsigned int pos, unsigned int cl)
{
    cl=Fclast[cl-(Bclast&(0xffff*ClBufSiz))];  	// zaporedje clastrov datoteke
    return RacClastS(cl)+((pos>>9)&(FATsnc-1)); // 512 bytes/sector
}

unsigned int ReadFAT(unsigned int cl)
{
    return	MMCreadSect(FATfat1+(cl>>8)); // 512 bytes/sector
}

unsigned int PosCl(void)
{
    return (FrdPos/FATsnc)>>9; 	 // 512 bytes/sector
}

//=====================
// iskanje verige datoteke
int FnextCl(void)
{
    unsigned int *i,er,cl,cn;
    unsigned int Nclast;
    
    Nclast=RNclast(Fsize); //stevilo vseh clastrov datoteke
    
    i=&Fclast[0];
    cl=Fclast[Bclast&(ClBufSiz-1)];  	// zaporedje clastrov datoteke
    if (Bclast==0)
        i++; 	// i=1; zacetek verige
    //	else
    //		i=0; 	// nadaljevanje verige 
    //	for (;i<ClBufSiz;i++)
    do {
        if (Bclast+1>=Nclast)
            return 0; 	// konec verige
        //		er=MMCreadSect(FATfat1+(cl>>8)); // 512 bytes/sector
        er=ReadFAT(cl);
        if (er)
            return (er);
        cn=BufT[cl&0xff]; 	// naslednji
        if ((cn-2)>=(0xfff7-2))
        {
            /*			er=MMCreadSect(FATfat2+(cl>>8)); // 512 bytes/sector
            if (er)
            return (er);
            cn=BufT[cl&0xff]; 	// naslednji
            if ((cn-2)>=(0xfff7-2))
            { */
            return -1; 	// predcasen konec verige
            /*			}
			else	// popravek FAT1
            {
            er=MMCwriteSect(FATfat1+(cl>>8),0); // 512 bytes/sector
            if (er)
            return (er);
        } */
        }
        cl=cn;
        //		Fclast[i]=cl;  	// zaporedje clastrov datoteke
        *i++=cl;  	// zaporedje clastrov datoteke
        Bclast++; 		// zadnji claster v bufferju
        //		i++;
        //	} while (i<ClBufSiz);
    } while (i<&Fclast[ClBufSiz]);
    return 0;
}
//=====================
// iskanje verige datoteke
int FnextClX(void)
{
    unsigned int i,er,cl,cn;
    unsigned int Nclast;
    
    Nclast=RNclast(Fsize); //stevilo vseh clastrov datoteke
    
    cl=Fclast[Bclast&(ClBufSiz-1)];  	// zaporedje clastrov datoteke
    if (Bclast)
        i=0; 	// nadaljevanje verige
    else
        i=1; 	// zacetek verige
    for (;i<ClBufSiz;i++)
    {
        if (Bclast+1>=Nclast)
            return 0; 	// konec verige
        er=MMCreadSect(FATfat1+(cl>>8)); // 512 bytes/sector
        if (er)
            return (er);
        cn=BufT[cl&0xff]; 	// naslednji
        if ((cn-2)>=(0xfff7-2))
        {
            /*			er=MMCreadSect(FATfat2+(cl>>8)); // 512 bytes/sector
            if (er)
            return (er);
            cn=BufT[cl&0xff]; 	// naslednji
            if ((cn-2)>=(0xfff7-2))
            { */
            return -1; 	// predcasen konec verige
            /*			}
			else	// popravek FAT1
            {
            er=MMCwriteSect(FATfat1+(cl>>8),0); // 512 bytes/sector
            if (er)
            return (er);
        } */
        }
        cl=cn;
        Fclast[i]=cl;  	// zaporedje clastrov datoteke
        Bclast++; 		// zadnji claster v bufferju
    }
    return 0;
}
//=====================

// odpiranje datoteke dolzine 'size' od clastra 'cl'
int Fopen(unsigned int cl, unsigned long size)
{
    Zclast=cl; 	// zacetni claster datoteke
    Bclast=0; 		// zadnji claster v bufferju
    FrdPos=0; 		// pozicija za branje iz datoteke
    Fclast[0]=cl;  	// zaporedje clastrov datoteke
    if (cl==0)
    {
        Fsize=(FATdat-FATdir)<<9;	// max dolzina root direktorija
        return 0;
    }
    Fsize=size; 	// dolzina datoteke
    /*	if (size>0)
    Nclast=(((size-1)>>9)/FATsnc)+1; //stevilo vseh clastrov datoteke
    else
    return Nclast=0; 	// prazna datoteka */
    if (size==0)
        return 0;
    return FnextCl();
}
/*
unsigned char izniz(unsigned char *buf,unsigned int n)
{
return buf[n];
}

void LcopyI(unsigned long src, unsigned int *dest)
{
dest[0]=src;
dest[1]=src>>16;
}
*/
/*
void Bcopy(unsigned int *src, unsigned int *dest, unsigned int n)
{
unsigned int i;
//	for (i=0;i<n;i++)
i=n;
do {
*dest++=*src++;
	} while (--i);
} */

void Bfill(unsigned int *buf, unsigned int Len, unsigned int val)
{
    //	unsigned int i;
    //	for (i=0;i<Len;i++)
    do {
        *buf++=val;
    } while (--Len);
}

// branje sektorja za pozicijo pos
int Fseek(unsigned long pos)
{
    unsigned int cl,er;
    
    if (pos>=Fsize) 	// pozicija za branje iz datoteke
        return -1; 	// konec datoteke
    FrdPos=pos; 	// pozicija za branje iz datoteke
    if (Fclast[0]==0)	// root direktorij
    {
        er=MMCreadSect(FATdir+((unsigned int)FrdPos>>9)); // 512 bytes/sector
    }
    else
    {
        //		cl=FrdPos/(FATsnc<<9); 	 // 512 bytes/sector MAX 64k claster!
        //		cl=(FrdPos/FATsnc)>>9; 	 // 512 bytes/sector
        cl=PosCl();   // trenutni claster
        if (cl<(Bclast&(0xffff*ClBufSiz)))    // pomik nazaj
        {
            Bclast=0; 		// zadnji claster v bufferju
            Fclast[0]=Zclast;  	// zaporedje clastrov datoteke
            er=FnextCl();
            if ((int)er>0)
                return (-3); 	// napaka pri branju verige
        }
        while (cl>Bclast) 		// zadnji claster v bufferju
        {
            er=FnextCl();
            if ((int)er>0)
                return (-3); 	// napaka pri branju verige
            if ((int)er==-1)	// predcasen konec verige
                if (cl>Bclast) 		// zadnji claster v bufferju
                    return -1; 	// konec datoteke
        }
        //		cl=Fclast[cl-(Bclast&(0xffff*ClBufSiz))];  	// zaporedje clastrov datoteke
        //		er=MMCreadSect(FATdat+(unsigned long)(cl-2)*FATsnc+((FrdPos>>9)&(FATsnc-1))); // 512 bytes/sector
        er=MMCreadSect(RacSect(FrdPos,cl)); // 512 bytes/sector
    }
    if (er)
        return (-2); 	// napaka pri branju
    return pos&0x1ff;  // pozicija v BufT
}

// branje enega byta iz odprte datoteke
int Fget(void)
{
    int i;
    
    i=Fseek(FrdPos);
    if (i<0) 	// pozicija za branje iz datoteke
        return -1; 	// konec datoteke
    i=((unsigned char *)&BufT)[i];
    FrdPos++;
    return i;  // znak iz datoteke
}

/*
int LookSubDir(unsigned int cl, unsigned int *name, int NN, unsigned int *ZC, unsigned long *Siz)
{
// iskanje NN datotek z imeni name[] v poddirektoriju 'cl'
// rezulati: ZC[] zacetni claster, Siz[] dolzina
int i,n,f,r;
unsigned int c;
unsigned long s;

for (i=0;i<NN;i++)
{
ZC[i]=0;
Siz[i]=0;
	}
r=0;
Bclast=0; 		 // zadnji claster v bufferju
// 	Nclast=ClBufSiz; // stevilo vseh clastrov datoteke (najvecja vrednost)
Fsize=(long)(ClBufSiz*FATsnc)<<9; //stevilo vseh clastrov datoteke
Fclast[0]=cl;  	 // zaporedje clastrov datoteke
FnextCl(); 		// zakjucek z -1 (predcasen konec verige)
for (n=0;n<=Bclast;n++)
{
c=Fclast[n];
s=FATdat+(unsigned long)(c-2)*FATsnc;
for (c=0;c<FATsnc;c++)
{
if (MMCreadSect(s++))
return 	r; 	// napaka pri branju
for (f=0;f<NN;f++)
{
i=FindFile(BufT, &name[f*6]);
if (i>=0)
{
ZC[f]=BufT[i+13]; 	// zacetni claster datoteke
Siz[f]=((unsigned long)BufT[i+15]<<16)+BufT[i+14]; 	// dolzina datoteke
r++;
if (r==NN)
return r; //konec iskanja
				}
			}
		}
	}
return r;
}
*/
//===========================================================================

unsigned int LookFDir(unsigned int cl, unsigned int *name, unsigned long *Siz)
{
    // iskanje datoteke z imenom name[] v direktoriju 'cl'
    int i;
    int c;
    //	unsigned long s;
    
    //	Siz[0]=0;
    if (Fopen(cl, (long)ClBufSiz<<9)>0) 	// odpiranje datoteke od clastra 'cl'
        return 0xffff; 	// napaka pri branju
    //	s=0;
    FrdPos=0;
    do {
        //		FrdPos=s; 		// pozicija za branje iz datoteke
        c=Fget();
        if (c>0)
        {
            i=FindFile((unsigned char *)&BufT, (unsigned char *)&name[0]);
            if (i>=0)
            {
                //			Siz[0]=((unsigned long)BufT[i+15]<<16)+BufT[i+14]; 	// dolzina datoteke
                Bcopy((unsigned int *)&BufT[i+14], (unsigned int *)&Siz[0], 2);
                return BufT[i+13]; 	// zacetni claster datoteke  //konec iskanja
            }
        }
        //  	        s+=512;	// naslednji sektor v direktoriju
        FrdPos+=512-1;	// naslednji sektor v direktoriju (+1 v Fget!!!)
    } while (c>0);
    return 0xffff;
}

unsigned int RtotClast(void)
{
    return (FATtotSect-FATdat)/FATsnc+2; 		// stevilo vseh clastrov
}
//===========================================================================
// unsigned int FATfreCL[FATBufSiz]; 	// vreiga prostih clastrov
// iskanje n prostih clastrov v FAT1
unsigned int RFATfree(unsigned int n)
{
    unsigned int i;
    //	unsigned int nc;
    unsigned int cl,nc,*b;
    
    /*	if (n>FATBufSiz)
    n=FATBufSiz;	// omejitev verige clastrov
    */
    //	nc=(FATtotSect-FATdat)/FATsnc+2; 		// stevilo vseh clastrov
    nc=RtotClast();  // stevilo vseh clastrov
    i=0;
    /*	do {
    if (FATfreL>=nc)
    return i;
    FATfreL++; 		// zadnji prost claster
    //		if (MMCreadSect(FATfat1+(FATfreL>>8)))
    if (ReadFAT(FATfreL))
    return 0; 	// napaka pri branju
    if ((BufT[FATfreL&0xff]==0))	// ni zaseden claster
    {
    FATfreCL[i++]=FATfreL; 	// vreiga prostih clastrov
}
} while (i<n);
    return i;
    */
    cl=FATfreL&0xff00;
    if ((cl>=nc))
        return 0;
    while (cl<nc) {
        if (ReadFAT(cl))
            return 0; 	// napaka pri branju
        b=&BufT[0];
        do {
            if (*b++==0)	// nezaseden claster
            {
                if (i<FATBufSiz)
                    FATfreCL[i]=cl; 	// vreiga prostih clastrov
                i++;
                if (i>=n)
                {
                    if (cl>=nc)
                        i-=cl-nc;
                    FATfreL=cl;
                    return i;
                }
            }
            cl++;
        } while (cl&0xff);
    }
    i-=cl-nc;
    FATfreL=cl;
    return i;
}

// iskanje prostih clastrov v FAT1
unsigned int ChkFATfree(void)
{
    //	unsigned int n,cl,nc,*b;
    
    if (MMCrInfo())	// nastavitev parametrov FAT16
        return 0; 	// napaka
    FATfreL=0;
    return RFATfree(0xffff);
    /*
    //	n=(FATtotSect-FATdat)/FATsnc+2; 		// stevilo vseh clastrov
    n=RtotClast();  // stevilo vseh clastrov
    nc=n;
    cl=0;
    //	for (cl=0; cl<n; cl+=256)
    do {
    //		if (MMCreadSect(FATfat1+(cl>>8)))
    if (ReadFAT(cl))
    return 0; 	// napaka pri branju
    b=&BufT[0];
    //		for (i=0;i<256;i++)
    do {
    //			if (BufT[i++])	// zaseden claster
    if (*b++)	// zaseden claster
    nc--;
    cl++;
} while (cl&0xff);
    //		cl+=256;
} while (cl<n);
    return nc;
    */
}

// nastavitev casov datoteke
void SetDirTime(unsigned int *buf)
{
    unsigned long t;
    
    buf[6]=0; 		// Reserved for NT (lo), ms (hi)
    t=DirTimeDate();
    buf[7]=t;	  // Creation Time & Date
    buf[8]=t>>16;	  // Creation Time & Date
    buf[9]=t>>16; 			// Last Access Date
    buf[10]=0; 		// Reserved for FAT32
    buf[11]=t;	  // Last Write Time & Date
    buf[12]=t>>16;	  // Last Write Time & Date
    /*
    LcopyI(t,&buf[7]); 		// Creation Time & Date
    buf[9]=t>>16; 			// Last Access Date
    buf[10]=0; 		// Reserved for FAT32
    LcopyI(t,&buf[11]); 		// Last Write Time & Date
    */
}

int DodVerigaF(unsigned int f, unsigned int i, unsigned int ncl)	// dodajanje clastra ncl za claster i na FAT1+f
{
    //	unsigned long s;
    unsigned int s;
    
    s=(FATfat1+f)+(i>>8);
    if (MMCreadSect(s))
        return 1; 	// napaka pri branju
    BufT[i&0xff]=ncl; 	// nadaljevanje verige
    if (ncl!=0xffff)
    {
        if ((i>>8)==(ncl>>8))
            BufT[ncl&0xff]=0xffff; 	// konec verige
    }
    else
        i=ncl;  // ni nadaljevanja verige
    if (MMCwriteSect(s,1))		// zapis sektorja 's' iz 'BufT'
        return 1; 	// napaka pri pisanju
    if ((i>>8)!=(ncl>>8))	// oznacevanje konca
    {
        return (DodVerigaF(f, ncl, 0xffff));   // konec verige
        /*
        s=(FATfat1+f)+(ncl>>8);
        if (MMCreadSect(s))
        return 2; 	// napaka pri branju
        BufT[ncl&0xff]=0xffff; 	// konec verige
        if (MMCwriteSect(s,1))		// zapis sektorja 's' iz 'BufT'
        return 2; 	// napaka pri pisanju
        */
    }
    return 0;
}			

int DodVeriga(unsigned int i, unsigned int ncl)	// dodajanje clastra ncl za claster i
{
    if (DodVerigaF(0, i, ncl))
	return 1; 	// napaka
    return DodVerigaF(FATfat2-FATfat1, i, ncl);
    /*  
    unsigned long s;
    
    s=FATfat1+(i>>8);
    if (MMCreadSect(s))
    return 1; 	// napaka pri branju
    BufT[i&0xff]=ncl; 	// nadaljevanje verige
    if ((i>>8)==(ncl>>8))
    BufT[ncl&0xff]=0xffff; 	// konec verige
    if (MMCwriteSect(s,1))		// zapis sektorja 's' iz 'BufT'
    return 1; 	// napaka pri pisanju
    s=FATfat2+(i>>8);
    if (MMCreadSect(s))
    return 1; 	// napaka pri branju
    BufT[i&0xff]=ncl; 	// nadaljevanje verige
    if ((i>>8)==(ncl>>8))
    BufT[ncl&0xff]=0xffff; 	// konec verige
    if (MMCwriteSect(s,1))		// zapis sektorja 's' iz 'BufT'
    return 1; 	// napaka pri pisanju
    if ((i>>8)!=(ncl>>8))	// oznacevanje konca
    {
    s=FATfat1+(ncl>>8);
    if (MMCreadSect(s))
    return 2; 	// napaka pri branju
    BufT[ncl&0xff]=0xffff; 	// konec verige
    if (MMCwriteSect(s,1))		// zapis sektorja 's' iz 'BufT'
    return 2; 	// napaka pri pisanju
    s=FATfat2+(ncl>>8);
    if (MMCreadSect(s))
    return 2; 	// napaka pri branju
    BufT[ncl&0xff]=0xffff; 	// konec verige
    if (MMCwriteSect(s,1))		// zapis sektorja 's' iz 'BufT'
    return 2; 	// napaka pri pisanju
}
    return 0;
    */
}			

// brisanje preostalih sektorjev v clastru
unsigned int ClrClast(unsigned long s)
{
    unsigned int i;
    i=FATsnc;
    while (--i)
    {
	s++;
	Bfill(BufT,256,0);
	if (MMCwriteSect(s,1))		// zapis sektorja 's' iz 'BufT'
            return i; 	// napaka pri pisanju
    }
    return i;
}

unsigned int MkFILE(unsigned int cl, unsigned int *name, unsigned long Len)
{
    unsigned int i,ncl,*b;
    int c;
    unsigned long s;
    
    FATfreL=0; 			// zadnji prost claster
    
    ncl=0;
    if (RFATfree(1)<1) 	// iskanje n prostih clastrov v FAT1
        return 0;		// ni prostora
    /*	if (NFreeCl==0xffff) 	// stevilo prostih clastrov ali ffff, ce ni prebrano
    NFreeCl=ChkFATfree();
    if (NFreeCl==0)		// ni prostora
    return 0; */
    ncl=FATfreCL[0];	// prost claster
    if (Fopen(cl, (long)ClBufSiz<<9)>0) 	// odpiranje datoteke od clastra 'cl'
        return 0; 	// napaka pri branju
    s=0;
    do {
        FrdPos=s; 		// pozicija za branje iz datoteke
        c=Fget();
        i=(s&0x1ff)>>1;
        if (c==0xe5) 	// zbrisana datoteka
        {
            c=0; 	// najden prostor
        }
        if (c)
            s+=32;	// naslednji zapis v direktoriju
    } while (c>0);
    s=BufSect;
    if (c)
    {
        if (cl)	// dodajanje clastra k direktoriju
        {
            i=Fclast[Bclast];  	// zaporedje clastrov datoteke
            if (DodVeriga(i,ncl))	// dodajanje clastra ncl za claster i
                return 0;  	// napaka
            //			s=FATdat+(unsigned long)(ncl-2)*FATsnc;
            s=RacClastS(ncl);   // izracun zacetnega sektorja clastra
            if (RFATfree(1)<1) 	// iskanje n prostih clastrov v FAT1
                return 0;		// ni prostora
            ncl=FATfreCL[0];	// prost claster
            /*			for (i=1;i<FATsnc;i++) 	// brisanje preostalih sektorjev v clastru
            {
            Bfill(BufT,256,0);
            if (MMCwriteSect(s+i,1))		// zapis sektorja 's' iz 'BufT'
            return 0; 	// napaka pri pisanju
        }
            */
            if (ClrClast(s)) // brisanje preostalih sektorjev v clastru
                return 0; 	// napaka pri pisanju
            /*                        
            i=FATsnc-1;
            s+=i;
            do {
            Bfill(BufT,256,0);
            if (MMCwriteSect(s,1))		// zapis sektorja 's' iz 'BufT'
            return 0; 	// napaka pri pisanju
            s--;
        } while (--i);
            */
            Bfill(BufT,256,0);
            i=0;	// 1. zapis v novem clastru
            //			BufSect=s;
        }
        else
            return 0;	// napaka
    }
    //	s=BufSect;
    b=&BufT[i];
    Bcopy(name, b, 6);	// ime direktorija/datoteke
    b[13]=ncl; 		// zacetni claster datoteke	
    //	LcopyI(Len,&BufT[i+14]); 	// dolzina datoteke lo,hi
    b[14]=Len; 	// dolzina datoteke lo
    b[15]=Len>>16; 	// dolzina datoteke hi
    SetDirTime(b); 		// nastavitev casov datoteke
    //                Send1buf((unsigned char *)&BufT, 512); // izpis bufferja
    if (MMCwriteSect(s,1))	// zapis sektorja 's' iz 'BufT'
        return 0; 	// napaka pri pisanju
    /*
    BufSect=0;  	// stevilka sektorja v bufferju
    if (MMCreadSect(s))
    return 0; 	// napaka pri branju
    */
    return ncl;
}

const unsigned int FNpika[6] = {'.'+256*' ',' '+256*' ',' '+256*' ',' '+256*' ',
' '+256*' ',' '+0x1000 };

unsigned int MkDIR(unsigned int cl, unsigned int *name)
{
    unsigned int ncl,*b;
    unsigned long s;
    
    FATfreL=0; 			// zadnji prost claster
    
    ncl=MkFILE(cl, name, 0);
    if (ncl==0)
        return 0;	// napaka
    // priprava praznega direktorija
    //	BufSect=0;
    Bfill(BufT,256,0);
    b=&BufT[0];
    do {
        Bcopy((unsigned int*)FNpika,b, 6);
        b[13]=ncl; 		// zacetni claster direktorija .
        //	LcopyI(0,&b[14]); 		// dolzina datoteke lo,hi
        SetDirTime(b); 		// nastavitev casov datoteke
        b+=16;
    } while (b<=&BufT[16]);
    //	Bcopy((unsigned int*)FNpika,&BufT[16], 6);
    BufT[16]='.'+256*'.';
    BufT[16+13]=cl; 	// zacetni claster direktorija ..
    //	LcopyI(0,&BufT[16+14]); 		// dolzina datoteke lo,hi
    //	SetDirTime(&BufT[16]); 		// nastavitev casov datoteke
    //	s=FATdat+(unsigned long)(ncl-2)*FATsnc;
    s=RacClastS(ncl);   // izracun zacetnega sektorja clastra
    if (MMCwriteSect(s,1))		// zapis sektorja 's' iz 'BufT'
        return 0; 	// napaka pri pisanju
    /*	for (i=1;i<FATsnc;i++) 	// brisanje preostalih sektorjev v clastru
    {
    Bfill(BufT,256,0);
    if (MMCwriteSect(s+i,1))		// zapis sektorja 's' iz 'BufT'
    return 0; 	// napaka pri pisanju
} */
    if (ClrClast(s)) // brisanje preostalih sektorjev v clastru
        return 0; 	// napaka pri pisanju
    // vpis v FAT1 in FAT2
    if (DodVeriga(ncl, 0xffff))	  // konec verige
        return 0; 	// napaka
    /*
    s=FATfat1+(ncl>>8);
    if (MMCreadSect(s))
    return 0; 	// napaka pri branju
    BufT[ncl&0xff]=0xffff; 	// konec verige
    if (MMCwriteSect(s,1))		// zapis sektorja 's' iz 'BufT'
    return 0; 	// napaka pri pisanju
    s=FATfat2+(ncl>>8);
    if (MMCreadSect(s))
    return 0; 	// napaka pri branju
    BufT[ncl&0xff]=0xffff; 	// konec verige
    if (MMCwriteSect(s,1))		// zapis sektorja 's' iz 'BufT'
    return 0; 	// napaka pri pisanju
    */
    return ncl;
}

// zapis sektorja iz 'BufT' v FAT1 in FAT2
unsigned int WriteFAT(unsigned int cc)
{
    if (MMCwriteSect(FATfat1+(cc>>8),0))		// zapis sektorja iz 'BufT' v FAT1
        return 0; 	// napaka pri pisanju
    if (MMCwriteSect(FATfat2+(cc>>8),1))		// zapis sektorja iz 'BufT' v FAT2
        return 0; 	// napaka pri pisanju
    return 1;
}

// zapis verige datoteke
int FgenClw(unsigned int cl, int n, unsigned int bb)
{
    unsigned int cc;
    int i,j;
    unsigned int nc;
    
    if (bb==0)
        return 0;
    bb--;
    if (bb)
    {
        nc=FATfreCL[bb];
        for (i=bb-1;i>=0;i--)
        {
            cc=FATfreCL[i];
            //			if (MMCreadSect(FATfat1+(cc>>8)))
            if (ReadFAT(cc))
                return 0; 	// napaka pri branju
            for (j=255;j>=0;j--)
                if ((BufT[j]==0))	// ni zaseden claster
                {
                    BufT[j]=nc;
                    nc=j+(cc&0xff00);
                    n--;
                    if (n<0)
                        return 0; 	// napaka 
                    if (nc==cl)
                        j=-1; 	// na zacetku
                }
            if (WriteFAT(cc)==0)  // zapis sektorja iz 'BufT' v FAT1 in FAT2
                return 0; 	// napaka pri pisanju
            /*			if (MMCwriteSect(FATfat1+(cc>>8),0))		// zapis sektorja iz 'BufT' v FAT1
            return 0; 	// napaka pri pisanju
            if (MMCwriteSect(FATfat2+(cc>>8),1))		// zapis sektorja iz 'BufT' v FAT2
            return 0; 	// napaka pri pisanju */
            if (nc==cl)
                i=-1; 	// na zacetku
        }
    }
    if (n>0) 	// 	zadnji sektor FAT
    {
        cc=FATfreCL[bb];
        //		if (MMCreadSect(FATfat1+(cc>>8)))
        if (ReadFAT(cc))
            return 0; 	// napaka pri branju
        do {
            nc=0;
            n--;
            if (n)
            {
                for (j=(cc&0xff)+1;j<256;j++)
                    if ((BufT[j]==0))	// ni zaseden claster
                    {
                        nc=j+(cc&0xff00);
                        j=256;
                    }
                if (nc)
                {
                    BufT[cc&0xff]=nc; 	// nadaljevanje verige
                    cc=nc;
                }
                else
                    return 0; 	// napaka
            }
            else
                BufT[cc&0xff]=0xffff; 	// konec verige
        } while (nc);
        return WriteFAT(cc);  // zapis sektorja iz 'BufT' v FAT1 in FAT2
        /*		if (MMCwriteSect(FATfat1+(cc>>8),0))		// zapis sektorja iz 'BufT' v FAT1
        return 0; 	// napaka pri pisanju
        if (MMCwriteSect(FATfat2+(cc>>8),1))		// zapis sektorja iz 'BufT' v FAT2
        return 0; 	// napaka pri pisanju */
    }
    return 1;
}

// generiranje verige datoteke
int FgenCl(unsigned int cl, unsigned int n)
{
    unsigned int cc;
    int i,ib,j;
    unsigned int nc;
    
    //	nc=(FATtotSect-FATdat)/FATsnc+2; 		// stevilo vseh clastrov
    nc=RtotClast();  // stevilo vseh clastrov
    cc=cl-1; 			// zadnji prost claster
    i=0;
    ib=0;
    j=0;	
    do {
        if (cc>=nc)
            return 0;	// ni dovolj prostora
        cc++; 			// zadnji prost claster
        //		if (MMCreadSect(FATfat1+(cc>>8)))
        if (ReadFAT(cc))
            return 0; 	// napaka pri branju
        if ((cc&0xff)==0)	// nov sektor
            j=0;
        if ((BufT[cc&0xff]==0))	// ni zaseden claster
        {
            if (j==0)	// samo prve v bloku
            {
                FATfreCL[ib++]=cc; 	// vreiga prostih clastrov
                if (ib>=FATBufSiz)	// poln buffer
                {
                    if (FgenClw(cl,i,ib)==0)
                        return -1; 	// napaka pri pisanju
                    cl=cc;
                    n-=i;
                    i=0;
                    ib=0;
                    FATfreCL[ib++]=cc; 	// vreiga prostih clastrov
                }
            }
            j++;
            i++;
        }
    } while (i<n);
    if (FgenClw(cl,i,ib)==0)
        return -1; 	// napaka pri pisanju
    return 1;
}

// brisanje verige datoteke
int FclrCl(unsigned int cl)
{
    unsigned int er;
    unsigned int cc;
    unsigned int nc;
    
    //	nc=(FATtotSect-FATdat)/FATsnc+2; 		// stevilo vseh clastrov
    nc=RtotClast()-2;  // stevilo vseh clastrov -2
    for (;;)
    {
        //		er=MMCreadSect(FATfat1+(cl>>8)); // 512 bytes/sector
        er=ReadFAT(cl);
        if (er)
            return (er);
        cc=cl;
        cl=BufT[cc&0xff]; 	// naslednji
        BufT[cc&0xff]=0;
        //		if ((cc>>8)!=(cl>>8))	// spremenjen sektor za nadaljevanje
        if (((cc^cl)>>8)!=0)	// spremenjen sektor za nadaljevanje
        {
            if (WriteFAT(cc)==0)  // zapis sektorja iz 'BufT' v FAT1 in FAT2
                return 1; 	// napaka pri pisanju
            /*			cc>>=8;
            if (MMCwriteSect(FATfat1+(cc),0))		// zapis sektorja iz 'BufT' v FAT1
            return 1; 	// napaka pri pisanju
            if (MMCwriteSect(FATfat2+(cc),1))		// zapis sektorja iz 'BufT' v FAT2
            return 1; 	// napaka pri pisanju */
        }
        //		if ((cl-2)>=(0xfff7-2))
        //		if ((cl-2)>=(nc-2))
        if ((cl-2)>=(nc))
            return 0; 	// konec verige
    }
}

// kreiranje datoteke dolzine 'size' od clastra 'cl'
int Fcreate(unsigned int cl, unsigned long size)
{
    int er;
    unsigned int Nclast;
    
    Bclast=0; 		// zadnji claster v bufferju
    FrdPos=0; 		// pozicija za branje iz datoteke
    Fclast[0]=cl;  	// zaporedje clastrov datoteke
    Fsize=size; 	// dolzina datoteke
    if (size>0)
        Nclast=RNclast(size); //stevilo vseh clastrov datoteke
    else
        return 0; 	// prazna datoteka
    er=FgenCl(cl,Nclast);
    if (er!=1)
        return -1; 	// napaka
    return FnextCl();
}

// brisanje datoteke 'name' z direktorija od clastra 'cl'
int Fdelete(unsigned int cl, unsigned int *name)
{
    int i;
    unsigned int Clast;
    unsigned long Siz;
    
    //	i=LookSubDir(cl,name,1,&Clast,&Siz);
    Clast=LookFDir(cl,name,&Siz);
    //	if (i) 	// obstaja
    if (Clast!=0xffff) 	// obstaja
    {
        i=FindFile((unsigned char *)&BufT, (unsigned char *)name);
        if (i>=0)
        {
            BufT[i]=(BufT[i]&0xff00)|0xe5; 	// brisanje zacetnega znaka
            if (MMCwriteSect(BufSect,0))	// zapis sektorja 's' iz 'BufT'
                return 1; 	// napaka pri pisanju
            if (Clast)
                FclrCl(Clast); 	// brisanje verige datoteke
        }
    }
    return 0;
}

// zapis enega worda v kreirano datoteko
int FwriteW(unsigned int w)
{
    unsigned int cl,er,p;
    
    if (FrdPos>=Fsize) 	// pozicija za branje iz datoteke
        return -1; 	// konec datoteke
    //	cl=(FrdPos/FATsnc)>>9; 	 // 512 bytes/sector
    cl=PosCl();   // trenutni claster
    while (cl>Bclast) 		// zadnji claster v bufferju
    {
        er=FnextCl();
        if ((int)er>0)
            return (-3); 	// napaka pri branju verige
        if ((int)er==-1)	// predcasen konec verige
            if (cl>Bclast) 		// zadnji claster v bufferju
                return -1; 	// konec datoteke
    }
    p=FrdPos&0x1fe;
    //	if ((FrdPos&0x1fe)==0)	// nov sektor
    if (p==0)	// nov sektor
        Bfill(BufT,256,0xffff);	// ciscenje bufferja
    //	BufT[(FrdPos&0x1ff)>>1]=w;
    //	BufT[p>>1]=w;
    ((unsigned int *)&((unsigned char *)&BufT)[p])[0]=w;
    er=0;
    //	if ((FrdPos&0x1fe)==0x1fe)	// poln sektor
    if (p==0x1fe)	// poln sektor
        er=FwriteEnd();   // zapis sektorja
    FrdPos+=2;
    return er;
    /*	if ((FrdPos&0x1fe)==0)	// poln sektor
    {
    //		cl=Fclast[cl-(Bclast&(0xffff*ClBufSiz))];  	// zaporedje clastrov datoteke
    //		er=MMCwriteSect(FATdat+(unsigned long)(cl-2)*FATsnc+(((FrdPos-2)>>9)&(FATsnc-1)),1); // 512 bytes/sector
    er=MMCwriteSect(RacSect(FrdPos-2,cl),1); // 512 bytes/sector
    if (er)
    return (-2); 	// napaka pri pisanju
}
    return 0; */
}

// zapis zadnjega sektorja v kreirano datoteko
int FwriteEnd(void)
{
    unsigned int cl,er;
    
    if ((FrdPos&0x1fe)==0)	// nov sektor
        return 0; 		// prazen sektor - ni zapisovanja
    //	cl=(FrdPos/FATsnc)>>9; 	 // 512 bytes/sector
    cl=PosCl();   // trenutni claster
    //	cl=Fclast[cl-(Bclast&(0xffff*ClBufSiz))];  	// zaporedje clastrov datoteke
    //	er=MMCwriteSect(FATdat+(unsigned long)(cl-2)*FATsnc+(((FrdPos)>>9)&(FATsnc-1)),1); // 512 bytes/sector
    er=MMCwriteSect(RacSect(FrdPos,cl),1); // 512 bytes/sector
    if (er)
        return (-2); 	// napaka pri pisanju
    return 0;
}

//===========================================================================

void sek2clock(unsigned long s, unsigned char *t)
{
    unsigned int ure,sek;
    ure=s/3600;
    sek=s-ure*3600;
    t[3]=ure/24;
    t[2]=ure-t[3]*24;
    t[1]=sek/60;
    t[0]=sek-t[1]*60;
}



//===========================================================================
// tipkovnica
int TipkaVhod(void)
{
    int v;
    //  asm(" RPT #3*45 || NOP");	 // 3us zakasnitev (vsaj 2us brez dodatnega pul-up)
    v = P5IN;  
    v |= P5IN;
    v &= P5IN;
    //  P1OUT|=0x80;  // vse data linije na 1 (TIPKA4)
    P4OUT=0xff;  // vse data linije na 1
    
    P5DIR |=0x20; //P5.5 izhod
    P5OUT |=0x20; //P5.5
    
    v &=0x20; //P5.5
    P5DIR &=~0x20; //P5.5 vhod
    
    return v;
}
#define TkGor 0x10
#define TkLev 0x02
#define TkDes 0x08
#define TkEnt 0x04
#define TkDol 0x01
//#define TkRept (TkGor+TkDol+TkLev+TkDes)
//#define TkRept (TkGor+TkDol)

//#define Tmax 6
//const unsigned char Tkode[Tmax]={TkGor,TkDol,TkLev,TkDes,TkEnt,TkLev+TkDes};  
//const unsigned char Tchar[Tmax]={'+','-','<','>',13,'S'};  
#define Tmax 7
const unsigned char Tkode[Tmax]={TkGor,TkDol,TkLev,TkDes,TkEnt,TkLev+TkDes,TkGor+TkDol};  
const unsigned char Tchar[Tmax]={'+','-','<','>',13,'S','L'};  

//const unsigned int Tpiskov[Tmax]={0x5353,0x3333,0x3f3f,0xf0f0,0xff00,0x553c};

void BeriKey(void)
{
    unsigned char t,i;
    //  P5OUT&=~0x40;    // LE=0
    LE573hold();    // P4 za LCD in tipke
    //  P5OUT&=~0x20;  // vse data linije na 0 (TIPKA4)
    P4OUT=0;  // vse data linije na 0
    t=0;
    if (TipkaVhod()==0)
    {
        i=0x20;
        while (i)
        {
            P4OUT&=~i;  //posamezne data linije na 0
            if (TipkaVhod()==0){ 
              t|=i;
            }
            i>>=1;      
        }
        
        if (t & TkRept){
           LastKey=0; 	   // sprozi ponovitev tipke
           KeyBuf[0] = t;
        }
    }
     
    
    
//    if (LastKey != t)
//    {
//        LastKey = t;
//        //     Pisk=t>>1; 	//BEEP on (P6.6) ali off(t=0)
//        if (t)
//        {
//            for (i=0;i<Tmax;i++)
//                if (Tkode[i]==t)
//                {
//                    t=Tchar[i];   // zamenjava normalnih tipk
//                    //     	      BeepRol=Tpiskov[i];  // 16-bitni vzorec za pisk
//                }
//            if (KeyBuf[0])
//                KeyBuf[1]=t;
//            else
//                KeyBuf[0]=t;
//            TBCCTL6 = OUTMOD_7;   // PWM (beep ON)
//            //	Pisk=1; 	//BEEP on (P6.6)
//        }
//        //     else 
//        //        BeepRol=0;  // 16-bitni vzorec za pisk
//    }
//    else
        TBCCTL6 = OUTMOD_5;   // resst (beep OFF)
    //     if (t)
    //      BeepRol=0;	//BEEP off (P6.6)
    LE573set();   // P4 na 574 izhode
}


//desifriraj tipke
int KGet(char tipke)
{
  
    char t = KeyBuf[0];
    if((t & tipke) == tipke){
      KeyBuf[0] &= ~tipke;
      return 1;
    }
    return 0;
    
}
    
    
//    if ((t=KeyBuf[0])!=0)
//    {
//        KeyBuf[0]=KeyBuf[1];
//	KeyBuf[1]=0;
//    }
    
    //return t;


//===========================================================================

void LCD_cmd(unsigned char d)
{
    P5OUT&=~0x0c;    //RS=0, wr=0 (RW=0)
    P5OUT|=0x02;    //rd=1 (E=1)
    P4OUT=d;
    P5OUT&=~0x10;   // >CS=0
    P5OUT|=0x10;    // >CS=1
    //      P7OUT|=0x41;    // wr=1, >CS=1
}

void LCD_write(unsigned char d)
{
    P5OUT|=0x0a;    //RS=1, rd=1 (E=1)
    P4OUT=d;
    P5OUT&=~0x04;   // wr=0 (RW=0)
    P5OUT&=~0x10;   // >CS=0
    P5OUT|=0x10;    // >CS=1
    //      P7OUT|=0x41;    // wr=1, >CS=1
}

char LCD_read(int rs)
{
    char d;
    if (rs)
        P5OUT|=0x08;    //RS=0
    else
        P5OUT&=~0x08;   //RS=0
    P5OUT|=0x04;     // wr=1 (RW=1)
    P4DIR = 0x00;   // P4 vhodi
    P5OUT&=~0x02;    // rd=0 (E=0)
    P5OUT&=~0x10;   // >CS=0
    d=P4IN;
    P5OUT|=0x10;    // >CS=1
    P4DIR = 0xff;   // P4 izhodi
    return d;
}

void LCD_init(void)
{
    int i;
    P5OUT&=~0x01;    // LCDreset = 0
    LE573hold();    // P4 za LCD in tipke
    P5OUT|=0x80;   // CSflash=1
                  //   P2OUT &=~0x80;                             // P2.7 LCDreset = 0
    KeyBuf[0]=KeyBuf[1]=0; // shranjene tipke
    P5OUT|=0x1f;    // wr=1, >CS=1 , E=1, RS=1 , LCDreset = 1
    //   LCD_cmd(0xe2);  // (14) reset
    //	LCD_fill(0);  //brisanje LCD
    i=LCD_read(0);
    if (i&0x80)  
        i=LCD_read(0);
    LCD_cmd(0xa2);  // (11) LCD bias 1/9
    LCD_cmd(0xa0);  //(8)  ADC selection normal
    LCD_cmd(0xc0);  //(15) Common output normal
    //   LCD_cmd(0x27);  //(17) V5 (Rb/Ra)=7 (max)
    LCD_cmd(0x26);  //(17) V5 (Rb/Ra)=6
    //	LCDkontrast=0x20; // LCD kontrast
    LCD_cmd(0x81); LCD_cmd(0x20-6);  //(18) SET Kontrast 50% Powertip
    //   LCD_cmd(0x81); LCD_cmd(0x1c);  //(18) SET Kontrast 43.75%
    //   LCD_cmd(0x81); LCD_cmd(0x18);  //(18) SET Kontrast 37.5%
    LCD_cmd(0x2F);  //(21) Power Saver OFF
    LCD_cmd(0xaF);  //(1) LCD ON
    
    LCD_cmd(0xa6);  //(9) LCD neinverzni
    //   LCD_cmd(0xa7);  //(9) LCD inverzni
    
    //   LCD_cmd(0xa5);  //(10) LCD all ON
    
    LCD_cmd(0x0040);  // (2)  Disply start line
    LCD_cmd(0x00A4);  // (10) Disply all pos16s OFF (normal)
    //   LCD_cmd(0x00E0);  // (12) Read modify write
    //   LCD_cmd(0x00EE);  // (13) End Read modify write
    //   LCD_cmd(0x002F);  // (16) Power control set
    LCD_cmd(0x00AC);  // (19)  Static indicator OFF
    LCD_cmd(0x00F0);  // (22) Test (konec testa)
    LE573set();   // P4 na 574 izhode
}

// Prenos LCD[i]  vsebine LCD prikazovalnika na LCD

void LCD_sendC(void)
{
    unsigned int i,p;
    //  P5OUT&=~0x40;    // LE=0
    LE573hold();    // P4 za LCD in tipke
    //  LCD_cmd(0x81); LCD_cmd(LCDkontrast);  //(18) SET Kontrast 
    LCD_cmd(0x0040);  // (2)  Disply start line
#if VertLCD
    // pokoncna postavitev LCD
    p=0xb7;  //Page 7 + konanda 0xb0
#else
    // lezeca postavitev LCD
    p=0xb0;  //Page 0 + konanda 0xb0
#endif
    for (i=0;i<1024;i++)
    {
        if ((i&0x7f)==0)
        {
            LCD_cmd(0x00); LCD_cmd(0x10); // (4) Column Address = 0
#if VertLCD
            // pokoncna postavitev LCD
            LCD_cmd(p--); // (3) Page
#else
            // lezeca postavitev LCD
            LCD_cmd(p++); // (3) Page
#endif
            P5OUT|=0x0a;    //RS=1, rd=1
            P5OUT&=~0x04;       // wr=0
        }
        //   LCD_write(LCD[i]);   // vsebina LCD prikazovalnika
        P4OUT=LCD[i];
        //      P7OUT&=~0x40;   // wr=0
        P5OUT&=~0x10;   // >CS=0
        P5OUT|=0x10;    // >CS=1
        //      P7OUT|=0x41;    // wr=1, >CS=1
    }
    //  P4OUT=0xFF;  //vse data linije na 1
    LE573set();   // P4 na 574 izhode
}


void LCD_fill(unsigned char d)
{
    int i;
    unsigned int dd;
    unsigned int *L;
    L=(unsigned int*)&LCD[0];
    dd=d;
    dd<<=8;
    dd|=d;
    /*  for (i=1023;i>=0;i--)
    LCD[i]=d; */
    for (i=511;i>=0;i--)
    {
        L[0]=dd;
        L++;
    }
    GrX=GrY=0;   // koordinati tocke na LCD
    inv=0;   // normalen izpis
}
/*
void rutine1(void)   // obdelovalne rutine brez LCD izpisa
{
}
*/
//===========================================================================
#if VertLCD
// pokoncna postavitev LCD
void plot(s16  x, s16  y)
{
    // tocka (0,0) je zgoraj levo
    if (x>=0)
        if (x<64)
            if (y>=0)
                if (y<128)
                    //    	  LCD[127-x+((y&0x38)<<4)] |= 0x80 >> (y&7);
                    //    	  LCD[127-y+((x&0x38)<<4)] |= 0x80 >> (x&7);
                    LCD[y+(((~x)&0x38)<<4)] |= 0x01 << (x&7);
    GrX=x;
    GrY=y;  // nova referencna tocka
}

void line(s16 x, s16 y)
{
    int dx,dy,x0,y0,s,tx,ty;
    if (x<0) x=-1;
    if (x>64) x=64;
    if (y<0) y=-1;
    if (y>128) y=128;
    x0=GrX;
    y0=GrY;
    dx=x-x0;
    if (dx<0)
        dx=-dx;
    dy=y-y0;
    if (dy<0)
        dy=-dy;
    tx=1;
    ty=1;
    if (x<x0) tx=-1;
    if (y<y0) ty=-1;
    if (dx>=dy)
    {
        s=dy>>1;
        while (x0!=x)
        {
            plot(x0, y0);
            s+=dy;
            if (s>=dx)
            {
                s-=dx;
                y0+=ty;
            }
            x0+=tx;
        }
    }
    else
    {
        s=dx>>1;
        while (y0!=y)
        {
            plot(x0, y0);
            s+=dx;
            if (s>=dy)
            {
                s-=dy;
                x0+=tx;
            }
            y0+=ty;
        }
    }
    plot(x, y);
    
    /*
    dx=x-x0;
    dy=y-y0;
    if (abs(dx)>=abs(dy))
    {
    if (dx>0)
    for (i=0;i<=dx;i++)
    plot(x0+i, y0+i*dy/dx);
    else
    for (i=0;i>=dx;i--)
    plot(x0+i, y0+i*dy/dx); 
}
    else
    {
    if (dy>0)
    for (i=0;i<=dy;i++)
    plot(x0+i*dx/dy,y0+i);
    else
    for (i=0;i>=dy;i--)
    plot(x0+i*dx/dy,y0+i);
} */
    
}

void liner(s16 x, s16 y)
{
    line(GrX+x, GrY+y);
}

void hline(s16 y)
{
    plot(0,y);
    liner(63,0);
    GrX=0; GrY=y+2;   // koordinati tocke na LCD
}

void put1RS(unsigned char c);  // izpis znaka na PC RS232

int putchar(int a)
{
    switch(OutDev)   // izhodna enota 0=LCD, 8=PC
    {
    case 0: GRchar8(a);   // ; normalni znaki
    break;
    case 1: GRchar12(a);  // ; srednji znaki
    break;
    case 2: GRchar18(a);  // ; veliki znaki
    break;
    case 4:
    case 8: SendRS1(a);  // izpis znaka na PC RS232
    break;
    }
    return a;
}
//===========================================================================
#else
// lezeca postavitev LCD

void plot(s16 x, s16 y)
{
    // tocka (0,0) je zgoraj levo
    if (x>=0)
  	if (x<128)
            if (y>=0)
   	  	if (y<64)
                    //    	  LCD[127-x+((y&0x38)<<4)] |= 0x80 >> (y&7);  //obrnjene data linije
                    LCD[127-x+((y&0x38)<<4)] |= 0x1 << (y&7);
    GrX=x;
    GrY=y;  // nova referencna tocka
}

void line(s16 x, s16 y)
{
    s16 dx,dy,x0,y0,s,tx,ty;
    if (x<0) x=-1;
    if (x>128) x=128;
    if (y<0) y=-1;
    if (y>64) y=64;
    x0=GrX;
    y0=GrY;
    dx=x-x0;
    if (dx<0)
        dx=-dx;
    dy=y-y0;
    if (dy<0)
        dy=-dy;
    tx=1;
    ty=1;
    if (x<x0) tx=-1;
    if (y<y0) ty=-1;
    if (dx>=dy)
    {
        s=dy>>1;
        while (x0!=x)
        {
            plot(x0, y0);
            s+=dy;
            if (s>=dx)
            {
                s-=dx;
                y0+=ty;
            }
            x0+=tx;
        }
    }
    else
    {
        s=dx>>1;
        while (y0!=y)
        {
            plot(x0, y0);
            s+=dx;
            if (s>=dy)
            {
                s-=dy;
                x0+=tx;
            }
            y0+=ty;
        }
    }
    plot(x, y);
    
    /*
    dx=x-x0;
    dy=y-y0;
    if (abs(dx)>=abs(dy))
    {
    if (dx>0)
    for (i=0;i<=dx;i++)
    plot(x0+i, y0+i*dy/dx);
    else
    for (i=0;i>=dx;i--)
    plot(x0+i, y0+i*dy/dx); 
}
    else
    {
    if (dy>0)
    for (i=0;i<=dy;i++)
    plot(x0+i*dx/dy,y0+i);
    else
    for (i=0;i>=dy;i--)
    plot(x0+i*dx/dy,y0+i);
} */
    
}

void liner(s16 x, s16 y)
{
    line(GrX+x, GrY+y);
}

void hline(s16 y)
{
    plot(0,y);
    liner(127,0);
    GrX=0; GrY=y+2;   // koordinati tocke na LCD
}

void boxr(s16 dx, s16 dy)
{
    liner(dx,0);
    liner(0,dy);
    liner(-dx,0);
    liner(0,-dy);
}

// izris horizontalnega bar grafa
void Hbar(s16 dx, u16 dy, s16 b)
{
    u16 i,g;
    
    g=GrX;
    boxr(dx,dy);
    for (i=1;i<dy;i++)
    {
        GrX=g;
        GrY++;
        liner(b,0);
    }
}

void SetInv(s16 fl)
{
    s16 g;
    
    if (fl)
    {
        PosInv=GrX; 	// pozicija zacetka inverznega izpisa
        inv=0x00ff; 	// za invertiran izpis tekstov
    }
    else
    {
        if (inv) 	// za invertiran izpis tekstov
        {
            inv=0; 	// za invertiran izpis tekstov
            if (PosInv<GrX)
            {
                g=GrX;
                GrY--;
                GrX--;
                if (PosInv>0)
                {
                    line(PosInv-1,GrY);
                    liner(0,8);
                    GrY-=7;
                }
                else
                {
                    line(0,GrY);
                    GrY++;
                }
                GrX=g;
            }
        }		
    }
}

void put1RS(unsigned char c);  // izpis znaka na PC RS232

int putchar(int a)
{
    switch(OutDev)   // izhodna enota 0=LCD, 8=PC
    {
    case 0: GRchar5(a);   // ; normalni znaki
    break;
    case 1: GRchar7(a);  // ; srednji znaki
    break;
    case 2: GRchar12(a);  // ; veliki znaki
    break;
    case 4:
        //case 8: SendRS1(a);  // izpis znaka na PC RS232
        break;
    }
    return a;
}

#endif
//===========================================================================

//===========================================================================
#if VertLCD
// pokoncna postavitev LCD
const unsigned char CRKE8[];
const unsigned char CRKE12[];
const unsigned int CRKE18[];

void GRchar8(unsigned char c)
{
    unsigned int i,t,d,ay,rx,rx2;
    //  unsigned int zn[8];  // pripravljen znak za shiftanje
    switch (c)
    {
    case 13:
        GrX=0;
        GrY+=8; 
        rutine1();   // obdelovalne rutine brez LCD izpisa     
        break;
    default:
        t=c-32;
        if (t<0x72)
        {
            t=t*8;
            if (GrX>64-5)
            {
                GrX=0;
                GrY+=8;
            }
            rx=(GrX&7);    // siftanje znaka desno
            ay=GrY;
            if (GrY<128)
            {
                ay+=(((~GrX)&0x38)<<4);  // naslov prve tocke
                i=128-GrY;
                if (i>8) i=8;
                if (rx<3)
                {
                    while (i--)
                        if (ay<1024)
                        {
                            d=CRKE8[t++] ^ inv;
                            LCD[ay++] |= d << rx;
                        }
                }
                else
                {
                    rx2=8-rx;
                    while (i--)
                        if (ay<1024)
                        {
                            d=CRKE8[t++] ^ inv;
                            LCD[ay++] |= d << rx;
                            if (ay>=129)
                                LCD[ay-129] |= d >> rx2;
                        }
                }
                GrX+=6;
            }     
        }
    }
}

void GRchar12(unsigned char c)
{
    unsigned int i,t,d,ay,rx,rx2;
    inv=0;   // normalen izpis
    switch (c)
    {
    case 13:
        GrX=0;
        GrY+=12; 
        rutine1();   // obdelovalne rutine brez LCD izpisa     
        break;
    default:
        t=c-32;
        if (t<0x75+2)
        {
            t=t*12;
            if (GrX>64-7)
            {
                GrX=0;
                GrY+=12;
            }
            rx=(GrX&7);    // siftanje znaka desno
            ay=GrY;
            if (GrY<128)
            {
                ay+=(((~GrX)&0x38)<<4);  // naslov prve tocke
                i=128-GrY;
                if (i>12) i=12;
                if (rx<1)
                {
                    while (i--)
                        if (ay<1024)
                        {
                            d=CRKE12[t++] ^ inv;
                            LCD[ay++] |= d << rx;
                        }
                }
                else
                {
                    rx2=8-rx;
                    while (i--)
                        if (ay<1024)
                        {
                            d=CRKE12[t++] ^ inv;
                            LCD[ay++] |= d << rx;
                            if (ay>=129)
                                LCD[ay-129] |= d >> rx2;
                        }
                }
                GrX+=8;
            }     
        }
    }
}

void GRchar18(unsigned char c)
{
    unsigned int i,t,d,ay,rx,rx2,zx3;
    inv=0;   // normalen izpis
    switch (c)
    {
    case 32:
        GrX+=14;
        break;
    case 13:
        GrX=0;
        GrY+=20; 
        rutine1();   // obdelovalne rutine brez LCD izpisa     
        break;
    case 1:  // ozka enka (sirine 7+2 tock
        GRchar12(0x95);  // zgornji del
        GrX-=8;
        GrY+=8;
        GRchar12(0x96);  // spodnji del
        GrY-=8;
        GrX++;
        break;
    case '-':
        c=58;  // znak - za znakom 9
    default:
        t=c-48;
        if (t<11)
        {
            t=t*18;
            if (GrX>64-12)
            {
                GrX=0;
                GrY+=20;
            }
            rx=(GrX&7);    // siftanje znaka desno
            ay=GrY;
            if (GrY<128)
            {
                ay+=(((~GrX)&0x38)<<4);  // naslov prve tocke
                i=128-GrY;
                if (i>18) i=18;
                rx2=8-rx;
                if (rx<5)
                {
                    while (i--)
                        if (ay<1024)
                        {
                            d=CRKE18[t++] ^ inv;
                            LCD[ay++] |= d << rx;
                            if (ay>=129)
                                LCD[ay-129] |= d >> rx2;
                        }
                }
                else
                {
                    zx3=16-rx;
                    while (i--)
                        if (ay<1024)
                        {
                            d=CRKE18[t++] ^ inv;
                            LCD[ay++] |= d << rx;
                            if (ay>=129)
                            {
                                LCD[ay-129] |= d >> rx2;
                                if (ay>=129+128)
                                    LCD[ay-129-128] |= d >> zx3;
                            }
                        }
                }
                GrX+=14;
            }     
        }
    }
}


// ;---------------
// TABCHAR:
// ; normalni znaki
// ;---------------
const unsigned char CRKE8[]={
    // ; h'20
    // ; presledek
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ; $00,$00,$5f,$00,$00 ; "!"
    0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04, 0x00,
    // ; $00,$07,$00,$07,$00 ; """
    0x0A, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ; $14,$7f,$14,$7f,$14 ; "#"
    0x0A, 0x0A, 0x1F, 0x0A, 0x1F, 0x0A, 0x0A, 0x00,
    // ; $24,$2a,$7f,$2a,$12 ; "$"
    0x04, 0x1E, 0x05, 0x0E, 0x14, 0x0F, 0x04, 0x00,
    // ; $63,$13,$08,$64,$63 ; "%"
    0x13, 0x13, 0x08, 0x04, 0x02, 0x19, 0x19, 0x00,
    // ; $30,$4e,$59,$26,$50 ; "&"
    0x06, 0x09, 0x05, 0x02, 0x15, 0x09, 0x16, 0x00,
    // ; $00,$04,$02,$01,$00 ; "'"
    0x06, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ; $00,$1c,$22,$41,$00 ; "("
    0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08, 0x00,
    // ; $00,$41,$22,$1c,$00 ; ")"
    0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02, 0x00,
    // ; $2a,$1c,$7f,$1c,$2a ; "*"
    0x04, 0x15, 0x0E, 0x1F, 0x0E, 0x15, 0x04, 0x00,
    // ; $08,$08,$3e,$08,$08 ; "+"
    0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00, 0x00,
    // ; $00,$40,$30,$00,$00 ; ","
    0x00, 0x00, 0x00, 0x00, 0x06, 0x04, 0x02, 0x00,
    // ; $08,$08,$08,$08,$08 ; "-"
    0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00,
    // ; $00,$60,$60,$00,$00 ; "."
    0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0x00,
    // ; $60,$10,$08,$04,$03 ; "/"
    0x10, 0x10, 0x08, 0x04, 0x02, 0x01, 0x01, 0x00,
    // ; h'30
    // ; $3e,$51,$49,$45,$3e ; "0"
    // 0x0E, 0x11, 0x19, 0x15, 0x13, 0x11, 0x0E, 0x00,
    0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x00,
    // ; $00,$42,$7f,$40,$00 ; "1"
    0x04, 0x06, 0x04, 0x04, 0x04, 0x04, 0x0E, 0x00,
    // ; $62,$51,$51,$49,$46 ; "2"
    0x0E, 0x11, 0x10, 0x08, 0x06, 0x01, 0x1F, 0x00,
    // ; $22,$41,$49,$49,$36 ; "3"
    0x0E, 0x11, 0x10, 0x0C, 0x10, 0x11, 0x0E, 0x00,
    // ; $18,$14,$12,$7f,$10 ; "4"
    0x08, 0x0C, 0x0A, 0x09, 0x1F, 0x08, 0x08, 0x00,
    // ; $27,$45,$45,$45,$3d ; "5"
    0x1F, 0x01, 0x0F, 0x10, 0x10, 0x11, 0x0E, 0x00,
    // ; $3c,$4a,$49,$49,$39 ; "6"
    0x0C, 0x02, 0x01, 0x0F, 0x11, 0x11, 0x0E, 0x00,
    // ; $01,$71,$09,$05,$03 ; "7"
    0x1F, 0x10, 0x08, 0x04, 0x02, 0x02, 0x02, 0x00,
    // ; $36,$49,$49,$49,$36 ; "8"
    0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E, 0x00,
    // ; $46,$49,$49,$29,$1e ; "9"
    0x0E, 0x11, 0x11, 0x1E, 0x10, 0x08, 0x06, 0x00,
    // ; $00,$00,$14,$00,$00 ; dvopicje
    0x00, 0x06, 0x06, 0x00, 0x06, 0x06, 0x00, 0x00,
    // ; $00,$40,$34,$00,$00 ; ";"
    0x00, 0x06, 0x06, 0x00, 0x04, 0x04, 0x02, 0x00,
    // ; $08,$14,$22,$41,$41 ; "<"
    0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x00,
    // ; $14,$14,$14,$14,$14 ; "="
    0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00, 0x00,
    // ; $41,$41,$22,$14,$08 ; ">"
    0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02, 0x00,
    // ; $02,$01,$51,$09,$06 ; "?"
    0x0E, 0x11, 0x10, 0x08, 0x04, 0x00, 0x04, 0x00,
    // ; h'40
    // ; $38,$44,$48,$30,$4c ; alfa
    0x00, 0x00, 0x12, 0x15, 0x09, 0x09, 0x16, 0x00,
    // ; $7e,$09,$09,$09,$7e ; "A"
    0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11, 0x00,
    // ; $41,$7f,$49,$49,$36 ; "B"
    0x0F, 0x11, 0x11, 0x0F, 0x11, 0x11, 0x0F, 0x00,
    // ; $3e,$41,$41,$41,$22 ; "C"
    0x0E, 0x11, 0x01, 0x01, 0x01, 0x11, 0x0E, 0x00,
    // ; $41,$7f,$41,$41,$3e ; "D"
    0x0F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0F, 0x00,
    // ; $7f,$49,$49,$49,$41 ; "E"
    0x1F, 0x01, 0x01, 0x0F, 0x01, 0x01, 0x1F, 0x00,
    // ; $7f,$09,$09,$09,$01 ; "F"
    0x1F, 0x01, 0x01, 0x0F, 0x01, 0x01, 0x01, 0x00,
    // ; $3e,$41,$41,$49,$3a ; "G"
    0x0E, 0x11, 0x01, 0x19, 0x11, 0x11, 0x0E, 0x00,
    // ; $7f,$08,$08,$08,$7f ; "H"
    0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11, 0x00,
    // ; $00,$41,$7f,$41,$00 ; "I"
    0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E, 0x00,
    // ; $20,$40,$41,$3f,$01 ; "J"
    0x1C, 0x08, 0x08, 0x08, 0x08, 0x09, 0x06, 0x00,
    // ; $7f,$08,$14,$22,$41 ; "K"
    0x11, 0x09, 0x05, 0x03, 0x05, 0x09, 0x11, 0x00,
    // ; $7f,$40,$40,$40,$40 ; "L"
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x1F, 0x00,
    // ; $7f,$02,$0c,$02,$7f ; "M"
    0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11, 0x00,
    // ; $7f,$04,$08,$10,$7f ; "N"
    0x11, 0x11, 0x13, 0x15, 0x19, 0x11, 0x11, 0x00,
    // ; $3e,$41,$41,$41,$3e ; "O"
    0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x00,
    // ; h'50
    // ; $7f,$09,$09,$09,$06 ; "P"
    0x0F, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x01, 0x00,
    // ; $3e,$41,$51,$21,$5e ; "Q"
    0x0E, 0x11, 0x11, 0x11, 0x15, 0x09, 0x16, 0x00,
    // ; $7f,$09,$19,$29,$46 ; "R"
    0x0F, 0x11, 0x11, 0x0F, 0x05, 0x09, 0x11, 0x00,
    // ; $26,$49,$49,$49,$32 ; "S"
    0x0E, 0x11, 0x01, 0x0E, 0x10, 0x11, 0x0E, 0x00,
    // ; $01,$01,$7f,$01,$01 ; "T"
    0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00,
    // ; $3f,$40,$40,$40,$3f ; "U"
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x00,
    // ; $07,$18,$60,$18,$07 ; "V"
    0x11, 0x11, 0x11, 0x0A, 0x0A, 0x04, 0x04, 0x00,
    // ; $7f,$20,$18,$20,$7f ; "W"
    0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A, 0x00,
    // ; $63,$14,$08,$14,$63 ; "X"
    0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11, 0x00,
    // ; $07,$08,$78,$08,$07 ; "Y"
    0x11, 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x00,
    // ; $61,$51,$49,$45,$43 ; "Z"
    0x1F, 0x10, 0x08, 0x04, 0x02, 0x01, 0x1F, 0x00,
    // ; $00,$7f,$41,$41,$00 ; "["
    0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E, 0x00,
    // ; $48,$7e,$49,$49,$42 ; "\"
    0x0C, 0x12, 0x02, 0x0F, 0x02, 0x02, 0x1F, 0x00,
    // ; $00,$41,$41,$7f,$00 ; "]"
    0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E, 0x00,
    // ; $04,$02,$7f,$02,$04 ; "^"
    0x04, 0x0E, 0x15, 0x04, 0x04, 0x04, 0x04, 0x00,
    // ; $08,$1c,$2a,$08,$08 ; "_"
    0x00, 0x04, 0x02, 0x1F, 0x02, 0x04, 0x00, 0x00,
    // ; h'60
    // ; $c0,$3e,$01,$49,$36 ; beta
    0x0C, 0x12, 0x12, 0x0A, 0x12, 0x12, 0x0A, 0x02,
    // ; $20,$54,$54,$7c,$40 ; "a"
    0x00, 0x00, 0x0E, 0x10, 0x1E, 0x11, 0x1E, 0x00,
    // ; $7f,$48,$44,$44,$38 ; "b"
    0x01, 0x01, 0x0D, 0x13, 0x11, 0x11, 0x0F, 0x00,
    // ; $38,$44,$44,$44,$44 ; "c"
    0x00, 0x00, 0x1E, 0x01, 0x01, 0x01, 0x1E, 0x00,
    // ; $38,$44,$44,$48,$7f ; "d"
    0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x1E, 0x00,
    // ; $38,$54,$54,$54,$08 ; "e"
    0x00, 0x00, 0x0E, 0x11, 0x0F, 0x01, 0x0E, 0x00,
    // ; $04,$7e,$05,$01,$02 ; "f"
    0x0C, 0x12, 0x07, 0x02, 0x02, 0x02, 0x02, 0x00,
    // ; $18,$a4,$a4,$a4,$78 ; "g"
    0x00, 0x00, 0x0E, 0x11, 0x11, 0x1E, 0x10, 0x0E,
    // ; $7f,$08,$04,$04,$78 ; "h"
    0x01, 0x01, 0x0D, 0x13, 0x11, 0x11, 0x11, 0x00,
    // ; $00,$44,$7d,$40,$00 ; "i"
    0x04, 0x00, 0x06, 0x04, 0x04, 0x04, 0x0E, 0x00,
    // ; $40,$80,$84,$7d,$00 ; "j"
    0x08, 0x00, 0x0C, 0x08, 0x08, 0x08, 0x09, 0x06,
    // ; $7f,$20,$10,$28,$44 ; "k"
    0x01, 0x01, 0x09, 0x05, 0x03, 0x05, 0x09, 0x00,
    // ; $00,$01,$7f,$40,$00 ; "l"
    0x06, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E, 0x00,
    // ; $7c,$04,$38,$04,$7c ; "m"
    0x00, 0x00, 0x0B, 0x15, 0x15, 0x15, 0x11, 0x00,
    // ; $7c,$08,$04,$04,$78 ; "n"
    0x00, 0x00, 0x0D, 0x13, 0x11, 0x11, 0x11, 0x00,
    // ; $38,$44,$44,$44,$38 ; "o"
    0x00, 0x00, 0x0E, 0x11, 0x11, 0x11, 0x0E, 0x00,
    // ; h'70
    // ; $fc,$24,$24,$24,$18 ; "p"
    0x00, 0x00, 0x0F, 0x11, 0x11, 0x0F, 0x01, 0x01,
    // ; $18,$24,$24,$24,$fc ; "q"
    0x00, 0x00, 0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10,
    // ; $7c,$08,$04,$04,$08 ; "r"
    0x00, 0x00, 0x0D, 0x13, 0x01, 0x01, 0x01, 0x00,
    // ; $48,$54,$54,$54,$20 ; "s"
    0x00, 0x00, 0x0E, 0x01, 0x0E, 0x10, 0x0F, 0x00,
    // ; $04,$3f,$44,$44,$20 ; "t"
    0x02, 0x02, 0x07, 0x02, 0x02, 0x12, 0x0C, 0x00,
    // ; $3c,$40,$40,$3c,$40 ; "u"
    0x00, 0x00, 0x11, 0x11, 0x11, 0x19, 0x16, 0x00,
    // ; $0c,$30,$40,$30,$0c ; "v"
    0x00, 0x00, 0x11, 0x11, 0x0A, 0x0A, 0x04, 0x00,
    // ; $3c,$40,$30,$40,$3c ; "w"
    0x00, 0x00, 0x11, 0x11, 0x15, 0x15, 0x0A, 0x00,
    // ; $44,$28,$10,$28,$44 ; "x"
    0x00, 0x00, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x00,
    // ; $4c,$50,$50,$3c,$00 ; "y"
    0x00, 0x00, 0x09, 0x09, 0x0E, 0x08, 0x06, 0x00,
    // ; $44,$64,$54,$4c,$44 ; "z"
    0x00, 0x00, 0x1F, 0x08, 0x04, 0x02, 0x1F, 0x00,
    // ; $38,$44,$45,$45,$44 ; c'
    0x0C, 0x00, 0x1E, 0x01, 0x01, 0x01, 0x1E, 0x00,
    // ; $48,$54,$55,$55,$24 ; s'
    0x0C, 0x00, 0x1E, 0x01, 0x0E, 0x10, 0x0F, 0x00,
    // ; $44,$65,$55,$4d,$44 ; z'
    0x0E, 0x00, 0x1F, 0x08, 0x04, 0x02, 0x1F, 0x00,
    // ; $4e,$71,$01,$71,$4e ; ohm
    0x0E, 0x11, 0x11, 0x11, 0x0A, 0x0A, 0x1B, 0x00,
    // ; $80,$7c,$40,$40,$3c ; mikro
    0x00, 0x00, 0x12, 0x12, 0x12, 0x12, 0x0E, 0x01,
    // ; h'80
    // ; ; 128
    0x00, 0x00, 0x1A, 0x15, 0x15, 0x0E, 0x04, 0x04,
    // ; ; 129 DB'
    0x1F, 0x00, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x00,
    // ; ; 130 DB'
    0x02, 0x0E, 0x10, 0x0E, 0x10, 0x0E, 0x02, 0x00,
    // ; ; 131 DB'
    0x04, 0x04, 0x1F, 0x00, 0x1F, 0x04, 0x04, 0x00,
    // ; ; 132 DB'
    0x00, 0x00, 0x00, 0x00, 0x04, 0x0A, 0x11, 0x1F,
    // ; ; 133 DB'
    0x00, 0x00, 0x00, 0x04, 0x04, 0x04, 0x0A, 0x11,
    // ; ; 134 DB'  stopinja
    0x07, 0x05, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ; h'87
    // ; ; CKAA DB'
    0x0A, 0x00, 0x0E, 0x11, 0x1F, 0x11, 0x11, 0x00,
    // ; ; CKA DB'
    0x08, 0x04, 0x0E, 0x11, 0x1F, 0x11, 0x11, 0x00,
    // ; ; CKE DB'
    0x08, 0x04, 0x1F, 0x01, 0x0F, 0x01, 0x1F, 0x00,
    // ; ; CKO DB'
    0x08, 0x04, 0x0E, 0x11, 0x11, 0x11, 0x0E, 0x00,
    // ; ; CKN DB'
    0x14, 0x0A, 0x11, 0x13, 0x15, 0x19, 0x11, 0x00,
    // ; ; CKEE DB'
    0x08, 0x04, 0x0E, 0x11, 0x1F, 0x01, 0x0E, 0x00,
    // ; CDC EQU H'8D'
    0x0E, 0x00, 0x0E, 0x01, 0x01, 0x11, 0x0E, 0x00,
    // ; CDS EQU H'8E'
    0x0E, 0x00, 0x0E, 0x01, 0x0E, 0x10, 0x0F, 0x00,
    // ; CDZ EQU H'8F'
    0x0E, 0x00, 0x1F, 0x08, 0x04, 0x02, 0x1F, 0x00,
    // ; ; H'90 (H'F6)'
    0x1F, 0x01, 0x02, 0x04, 0x02, 0x01, 0x1F, 0x00,
    // ; $08,$3c,$04,$3c,$04 ; pi
    0x00, 0x00, 0x1E, 0x0B, 0x0A, 0x0A, 0x00, 0x00,
    // ; - - -
};

// ;---------------
// MFONT:
// ;---------------
const unsigned char CRKE12[]={
    // ; h'20
    // ; presledek
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ; H'21 !
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x00, 0x0C, 0x0C, 0x00, 0x00,
    // ; H'22 "
    0x36, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ; H'23 #
    0x36, 0x36, 0x7F, 0x7F, 0x36, 0x36, 0x7F, 0x7F, 0x36, 0x36, 0x00, 0x00,
    // ; H'24 $
    0x08, 0x3E, 0x7F, 0x0B, 0x3F, 0x7E, 0x68, 0x7F, 0x3E, 0x08, 0x00, 0x00,
    // ; H'25 %
    0x67, 0x75, 0x37, 0x18, 0x18, 0x0C, 0x0C, 0x76, 0x57, 0x73, 0x00, 0x00,
    // ; H'26 &
    0x0E, 0x1F, 0x1B, 0x1B, 0x0F, 0x4E, 0x6F, 0x3B, 0x3F, 0x6E, 0x00, 0x00,
    // ; H'27 '
    0x1E, 0x1E, 0x18, 0x1C, 0x0E, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ; H'28 (
    0x30, 0x18, 0x0C, 0x06, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x30, 0x00, 0x00,
    // ; H'29 )
    0x06, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x30, 0x18, 0x0C, 0x06, 0x00, 0x00,
    // ; H'2a *
    0x00, 0x08, 0x49, 0x6B, 0x3E, 0x3E, 0x6B, 0x49, 0x08, 0x00, 0x00, 0x00,
    // ; H'2b +
    0x00, 0x0C, 0x0C, 0x0C, 0x3F, 0x3F, 0x0C, 0x0C, 0x0C, 0x00, 0x00, 0x00,
    // ; H'2c ,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x06, 0x00,
    // ; H'2d -
    0x00, 0x00, 0x00, 0x00, 0x3F, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ; H'2e .
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00, 0x00,
    // ; H'2f /
    0x60, 0x70, 0x30, 0x18, 0x18, 0x0C, 0x0C, 0x06, 0x07, 0x03, 0x00, 0x00,
    // ; H'30 0
    0x1C, 0x3E, 0x77, 0x63, 0x63, 0x63, 0x63, 0x77, 0x3E, 0x1C, 0x00, 0x00,
    // ; H'31 1
    0x1C, 0x1E, 0x1B, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00,
    // ; H'32 2
    0x3E, 0x7F, 0x63, 0x70, 0x38, 0x1C, 0x0E, 0x07, 0x7F, 0x7F, 0x00, 0x00,
    // ; H'33 3
    0x3E, 0x7F, 0x63, 0x60, 0x38, 0x78, 0x60, 0x63, 0x7F, 0x3E, 0x00, 0x00,
    // ; H'34 4
    0x30, 0x38, 0x3C, 0x3C, 0x36, 0x36, 0x7F, 0x7F, 0x30, 0x30, 0x00, 0x00,
    // ; H'35 5
    // 0x7F, 0x7F, 0x03, 0x03, 0x3F, 0x7F, 0x60, 0x63, 0x7F, 0x3E, 0x00, 0x00,
    0x7F, 0x7F, 0x03, 0x3F, 0x7F, 0x60, 0x60, 0x63, 0x7F, 0x3E, 0x00, 0x00,
    // ; H'36 6
    0x3E, 0x7F, 0x03, 0x03, 0x3F, 0x7F, 0x63, 0x63, 0x7F, 0x3E, 0x00, 0x00,
    // ; H'37 7
    0x7F, 0x7F, 0x30, 0x30, 0x18, 0x18, 0x0C, 0x0C, 0x06, 0x06, 0x00, 0x00,
    // ; H'38 8
    0x3E, 0x7F, 0x63, 0x63, 0x3E, 0x7F, 0x63, 0x63, 0x7F, 0x3E, 0x00, 0x00,
    // ; H'39 9
    0x3E, 0x7F, 0x63, 0x63, 0x7F, 0x7E, 0x60, 0x60, 0x7F, 0x3E, 0x00, 0x00,
    // ; H'3a :
    // 0x00, 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00, 0x00,
    0x00, 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x00,
    // ; H'3b ;
    0x00, 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x06, 0x00,
    // ; H'3c <
    0x20, 0x30, 0x18, 0x0C, 0x06, 0x06, 0x0C, 0x18, 0x30, 0x20, 0x00, 0x00,
    // ; H'3d =
    0x00, 0x00, 0x3E, 0x3E, 0x00, 0x00, 0x3E, 0x3E, 0x00, 0x00, 0x00, 0x00,
    // ; H'3e >
    0x02, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x18, 0x0C, 0x06, 0x02, 0x00, 0x00,
    // ; H'3f ?
    0x1E, 0x3F, 0x31, 0x30, 0x18, 0x0C, 0x0C, 0x00, 0x0C, 0x0C, 0x00, 0x00,
    // ; H'40 @
    0x3E, 0x7F, 0x60, 0x60, 0x6E, 0x6F, 0x6B, 0x6B, 0x7F, 0x3E, 0x00, 0x00,
    // ; H'41 A
    0x1C, 0x3E, 0x77, 0x63, 0x63, 0x63, 0x7F, 0x7F, 0x63, 0x63, 0x00, 0x00,
    // ; H'42 B
    0x3F, 0x7F, 0x63, 0x63, 0x7F, 0x3F, 0x63, 0x63, 0x7F, 0x3F, 0x00, 0x00,
    // ; H'43 C
    0x3C, 0x7E, 0x67, 0x03, 0x03, 0x03, 0x03, 0x67, 0x7E, 0x3C, 0x00, 0x00,
    // ; H'44 D
    0x1F, 0x3F, 0x73, 0x63, 0x63, 0x63, 0x63, 0x73, 0x3F, 0x1F, 0x00, 0x00,
    // ; H'45 E
    0x7F, 0x7F, 0x03, 0x03, 0x3F, 0x3F, 0x03, 0x03, 0x7F, 0x7F, 0x00, 0x00,
    // ; H'46 F
    0x7F, 0x7F, 0x03, 0x03, 0x3F, 0x3F, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00,
    // ; H'47 G
    0x3C, 0x7E, 0x67, 0x03, 0x03, 0x73, 0x73, 0x67, 0x7E, 0x3C, 0x00, 0x00,
    // ; H'48 H
    0x63, 0x63, 0x63, 0x63, 0x7F, 0x7F, 0x63, 0x63, 0x63, 0x63, 0x00, 0x00,
    // ; H'49 I
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x00, 0x00,
    // ; H'4a J
    0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x63, 0x7F, 0x3E, 0x00, 0x00,
    // ; H'4b K
    0x63, 0x33, 0x1B, 0x0F, 0x07, 0x07, 0x0F, 0x1B, 0x33, 0x63, 0x00, 0x00,
    // ; H'4c L
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x7F, 0x7F, 0x00, 0x00,
    // ; H'4d M
    0x63, 0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x63, 0x00, 0x00,
    // ; H'4e N
    0x63, 0x63, 0x67, 0x6F, 0x7F, 0x7B, 0x73, 0x63, 0x63, 0x63, 0x00, 0x00,
    // ; H'4f O
    0x3E, 0x7F, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x7F, 0x3E, 0x00, 0x00,
    // ; H'50 P
    0x3F, 0x7F, 0x63, 0x63, 0x7F, 0x3F, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00,
    // ; H'51 Q
    0x1C, 0x3E, 0x77, 0x63, 0x63, 0x63, 0x63, 0x7F, 0x3E, 0x6C, 0x00, 0x00,
    // ; H'52 R
    0x1F, 0x3F, 0x63, 0x63, 0x7F, 0x3F, 0x1F, 0x1B, 0x33, 0x63, 0x00, 0x00,
    // ; H'53 S
    0x3E, 0x7F, 0x63, 0x03, 0x3F, 0x7E, 0x60, 0x63, 0x7F, 0x3E, 0x00, 0x00,
    // ; H'54 T
    0x3F, 0x3F, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x00, 0x00,
    // ; H'55 U
    0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x7F, 0x3E, 0x00, 0x00,
    // ; H'56 V
    0x63, 0x63, 0x63, 0x63, 0x63, 0x77, 0x36, 0x3E, 0x1C, 0x08, 0x00, 0x00,
    // ; H'57 W
    0x63, 0x63, 0x6B, 0x6B, 0x6B, 0x6B, 0x6B, 0x7F, 0x3E, 0x14, 0x00, 0x00,
    // ; H'58 X
    0x63, 0x63, 0x36, 0x1C, 0x1C, 0x1C, 0x1C, 0x36, 0x63, 0x63, 0x00, 0x00,
    // ; H'59 Y
    0x33, 0x33, 0x33, 0x1E, 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x00, 0x00,
    // ; H'5a Z
    0x7F, 0x7F, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x7F, 0x7F, 0x00, 0x00,
    // ; H'5b [
    0x3E, 0x7F, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x7F, 0x3E, 0x00, 0x00,
    // ; H'5c "\"
    0x38, 0x3C, 0x36, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00,
    // ; H'5d ]
    0x3E, 0x7F, 0x63, 0x70, 0x38, 0x1C, 0x0E, 0x07, 0x7F, 0x7F, 0x00, 0x00,
    // ; H'5e ^
    0x3E, 0x7F, 0x63, 0x60, 0x38, 0x78, 0x60, 0x63, 0x7F, 0x3E, 0x00, 0x00,
    // ; H'5f _
    0x30, 0x38, 0x3C, 0x3C, 0x36, 0x36, 0x7F, 0x7F, 0x30, 0x30, 0x00, 0x00,
    // ; H'60 `
    0x06, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ; H'61 a
    0x00, 0x00, 0x3C, 0x7E, 0x60, 0x7E, 0x7F, 0x63, 0x7F, 0x7E, 0x00, 0x00,
    // ; H'62 b
    0x03, 0x03, 0x3F, 0x7F, 0x63, 0x63, 0x63, 0x63, 0x7F, 0x3F, 0x00, 0x00,
    // ; H'63 c
    0x00, 0x00, 0x3E, 0x7F, 0x63, 0x03, 0x03, 0x63, 0x7F, 0x3E, 0x00, 0x00,
    // ; H'64 d
    0x60, 0x60, 0x7E, 0x7F, 0x63, 0x63, 0x63, 0x63, 0x7F, 0x7E, 0x00, 0x00,
    // ; H'65 e
    0x00, 0x00, 0x3E, 0x7F, 0x63, 0x7F, 0x7F, 0x03, 0x7F, 0x3E, 0x00, 0x00,
    // ; H'66 f
    0x78, 0x7C, 0x0C, 0x3F, 0x3F, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x00, 0x00,
    // ; H'67 g
    0x00, 0x00, 0x7E, 0x7F, 0x63, 0x63, 0x63, 0x7F, 0x7E, 0x60, 0x7F, 0x3E,
    // ; H'68 h
    0x03, 0x03, 0x3F, 0x7F, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x00, 0x00,
    // ; H'69 i
    0x0C, 0x0C, 0x00, 0x0E, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x1E, 0x00, 0x00,
    // ; H'6a j
    0x30, 0x30, 0x00, 0x38, 0x38, 0x30, 0x30, 0x30, 0x30, 0x33, 0x3F, 0x1E,
    // ; H'6b k
    0x03, 0x03, 0x63, 0x33, 0x1B, 0x0F, 0x0F, 0x1B, 0x33, 0x63, 0x00, 0x00,
    // ; H'6c l
    0x0E, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x1E, 0x00, 0x00,
    // ; H'6d m
    0x00, 0x00, 0x3F, 0x7F, 0x6B, 0x6B, 0x6B, 0x6B, 0x63, 0x63, 0x00, 0x00,
    // ; H'6e n
    0x00, 0x00, 0x3F, 0x7F, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x00, 0x00,
    // ; H'6f o
    0x00, 0x00, 0x3E, 0x7F, 0x63, 0x63, 0x63, 0x63, 0x7F, 0x3E, 0x00, 0x00,
    // ; H'70 p
    0x00, 0x00, 0x3F, 0x7F, 0x63, 0x63, 0x63, 0x63, 0x7F, 0x3F, 0x03, 0x03,
    // ; H'71 q
    0x00, 0x00, 0x7E, 0x7F, 0x63, 0x63, 0x63, 0x63, 0x7F, 0x7E, 0x60, 0x60,
    // ; H'72 r
    0x00, 0x00, 0x36, 0x3E, 0x0E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x00, 0x00,
    // ; H'73 s
    0x00, 0x00, 0x3E, 0x7F, 0x03, 0x1E, 0x38, 0x60, 0x7F, 0x3E, 0x00, 0x00,
    // ; H'74 t
    0x00, 0x0C, 0x3F, 0x3F, 0x0C, 0x0C, 0x0C, 0x0C, 0x3C, 0x38, 0x00, 0x00,
    // ; H'75 u
    0x00, 0x00, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x7F, 0x7E, 0x00, 0x00,
    // ; H'76 v
    0x00, 0x00, 0x63, 0x63, 0x63, 0x36, 0x36, 0x1C, 0x1C, 0x08, 0x00, 0x00,
    // ; H'77 w
    0x00, 0x00, 0x63, 0x63, 0x6B, 0x6B, 0x6B, 0x7F, 0x3E, 0x14, 0x00, 0x00,
    // ; H'78 x
    0x00, 0x00, 0x63, 0x36, 0x1C, 0x1C, 0x1C, 0x1C, 0x36, 0x63, 0x00, 0x00,
    // ; H'79 y
    0x00, 0x00, 0x63, 0x63, 0x63, 0x36, 0x36, 0x3C, 0x18, 0x18, 0x18, 0x0C,
    // ; H'7a z
    0x00, 0x00, 0x7F, 0x7F, 0x30, 0x18, 0x0C, 0x06, 0x7F, 0x7F, 0x00, 0x00,
    // ; ZA IZPIS PREDZNAKA SREDNJE VELIKOSTI OB VELIKIH CIFRAH
    // ; H'7b { == ; H'2b +
    0x00, 0x0C, 0x0C, 0x0C, 0x3F, 0x3F, 0x0C, 0x0C, 0x0C, 0x00, 0x00, 0x00,
    // ; H'7c | == ; H'2c ,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x06, 0x00,
    // ; H'7d } == ; H'2d -
    0x00, 0x00, 0x00, 0x00, 0x3F, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ; H'7e ~
    0x00, 0x1C, 0x22, 0x41, 0x41, 0x41, 0x22, 0x1C, 0x00, 0x00, 0x00, 0x00,
    // ; H'7f 
    0x00, 0x1C, 0x3E, 0x7F, 0x7F, 0x7F, 0x3E, 0x1C, 0x00, 0x00, 0x00, 0x00,
    // ; H'80 €
    0x00, 0x00, 0x00, 0x1C, 0x3E, 0x36, 0x36, 0x3F, 0x1F, 0x06, 0x06, 0x06,
    // ; H'81 
    0x7F, 0x7F, 0x00, 0x63, 0x63, 0x63, 0x63, 0x63, 0x7F, 0x3E, 0x00, 0x00,
    // ; H'82 ‚
    0x03, 0x3F, 0x7F, 0x60, 0x3F, 0x3F, 0x60, 0x7F, 0x3F, 0x03, 0x00, 0x00,
    // ; H'83 ƒ
    0x0C, 0x0C, 0x0C, 0x3F, 0x3F, 0x00, 0x3F, 0x3F, 0x0C, 0x0C, 0x0C, 0x00,
    // ; H'84 „
    0x00, 0x00, 0x00, 0x08, 0x1C, 0x3E, 0x36, 0x63, 0x7F, 0x7F, 0x00, 0x00,
    // ; H'85 …
    0x00, 0x00, 0x00, 0x0C, 0x0C, 0x0C, 0x1E, 0x3F, 0x33, 0x21, 0x00, 0x00,
    // ; H'86 †
    0x1C, 0x3E, 0x36, 0x3E, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ; H'87 ‡
    0x63, 0x63, 0x1C, 0x3E, 0x77, 0x63, 0x63, 0x7F, 0x7F, 0x63, 0x00, 0x00,
    // ; H'88 ˆ
    0x18, 0x0C, 0x1C, 0x3E, 0x77, 0x63, 0x63, 0x7F, 0x7F, 0x63, 0x00, 0x00,
    // ; H'89 ‰
    0x18, 0x0C, 0x7F, 0x7F, 0x03, 0x1F, 0x1F, 0x03, 0x7F, 0x7F, 0x00, 0x00,
    // ; H'8a Š
    0x18, 0x0C, 0x3E, 0x7F, 0x63, 0x63, 0x63, 0x63, 0x7F, 0x3E, 0x00, 0x00,
    // ; H'8b ‹
    0x66, 0x33, 0x63, 0x67, 0x6F, 0x6F, 0x7B, 0x73, 0x73, 0x63, 0x00, 0x00,
    // ; H'8c Œ
    0x18, 0x0C, 0x3E, 0x7F, 0x63, 0x7F, 0x7F, 0x03, 0x7F, 0x3E, 0x00, 0x00,
    // ; H'8d 
    0x36, 0x1C, 0x3E, 0x7F, 0x63, 0x03, 0x03, 0x63, 0x7F, 0x3E, 0x00, 0x00,
    // ; H'8e Ž
    0x36, 0x1C, 0x3E, 0x7F, 0x03, 0x3F, 0x7E, 0x60, 0x7F, 0x3E, 0x00, 0x00,
    // ; H'8f 
    0x36, 0x1C, 0x7F, 0x7F, 0x30, 0x18, 0x0C, 0x06, 0x7F, 0x7F, 0x00, 0x00,
    // ; H'90 
    0x7F, 0x7F, 0x0E, 0x1C, 0x38, 0x1C, 0x0E, 0x07, 0x7F, 0x7F, 0x00, 0x00,
    // ; H'91 pi
    0x00, 0x00, 0x00, 0x3C, 0x16, 0x14, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ; - - -
    // ; H'92 gor
    // 0x00, 0x10, 0x10, 0x38, 0x38, 0x7C, 0x7C, 0xFE, 0xFE, 0x00, 0x00, 0x00,
    0x00, 0x08, 0x08, 0x1C, 0x1C, 0x3E, 0x3E, 0x7F, 0x7F, 0x00, 0x00, 0x00,
    // ; H'93 dol
    // 0x00, 0xFE, 0xFE, 0x7C, 0x7C, 0x38, 0x38, 0x10, 0x10, 0x00, 0x00, 0x00,
    0x00, 0x7F, 0x7F, 0x3E, 0x3E, 0x1C, 0x1C, 0x08, 0x08, 0x00, 0x00, 0x00,
    // ; H'94 srce
    // 0x00, 0x6C, 0xFE, 0xFE, 0x7C, 0x7C, 0x38, 0x10, 0x00, 0x00, 0x00, 0x00,
    // 0x6C, 0xFE, 0xFE, 0x7C, 0x7C, 0x38, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x36, 0x7F, 0x7F, 0x3E, 0x3E, 0x1C, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
    // H'95 zgornji del ozke velike enke
    // 0x0E, 0x1E, 0x3E, 0x7E, 0xFE, 0xDE, 0xCE, 0x0E, 0x0E, 0x0E, 0, 0,
    0x70, 0x78, 0x7C, 0x7E, 0x7F, 0x7B, 0x73, 0x70, 0x70, 0x70, 0, 0,
    // H'96 spodnji del ozke velike enke (y-8)
    // 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0, 0
    0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0, 0
        // ; - - -
};

// LFONT:
// ;---------------
const unsigned int CRKE18[]={
    // ; 0
    0x01F8, 0x03FC, 0x07FE, 0x070E, 0x070E, 0x0E07, 0x0E07, 0x0E07, 0x0E07, 0x0E07, 0x0E07, 0x0E07, 0x0E07, 0x070E, 0x070E, 0x03FC, 0x03FC, 0x01F8,
    // ; 1
    0x01C0, 0x01E0, 0x01F0, 0x01F8, 0x01FC, 0x01DC, 0x01CC, 0x01C0, 0x01C0, 0x01C0, 0x01C0, 0x01C0, 0x01C0, 0x01C0, 0x01C0, 0x01C0, 0x01C0, 0x01C0,
    // ;2
    0x01F8, 0x07FC, 0x07FE, 0x0F0F, 0x0E07, 0x0E07, 0x0E00, 0x0F00, 0x0780, 0x03C0, 0x01E0, 0x00F0, 0x0078, 0x003C, 0x001E, 0x0FFE, 0x0FFF, 0x0FFF,
    // ;3
    0x01F0, 0x07FC, 0x07FC, 0x0E1E, 0x0E0E, 0x0E00, 0x0F00, 0x07E0, 0x01E0, 0x07E0, 0x0F00, 0x0E00, 0x0E07, 0x0E07, 0x0F0E, 0x07FE, 0x03FC, 0x01F8,
    // ;4
    0x0380, 0x03C0, 0x03E0, 0x03E0, 0x03F0, 0x03B8, 0x03B8, 0x039C, 0x039C, 0x038E, 0x038F, 0x0FFF, 0x0FFF, 0x0FFF, 0x0380, 0x0380, 0x0380, 0x0380,
    // ;5
    0x07FC, 0x07FC, 0x07FE, 0x000E, 0x000E, 0x01FE, 0x03FE, 0x07FF, 0x070F, 0x0E00, 0x0E00, 0x0E00, 0x0E07, 0x0E07, 0x070E, 0x07FE, 0x03FC, 0x00F8,
    // ;6
    0x01F0, 0x07F8, 0x07FC, 0x0F1E, 0x0E0E, 0x0007, 0x0007, 0x01F7, 0x03FF, 0x07FF, 0x0F0F, 0x0E07, 0x0E07, 0x0E06, 0x0F0E, 0x07FE, 0x03FC, 0x01F0,
    // ;7
    0x0FFF, 0x0FFF, 0x0FFF, 0x0F00, 0x0780, 0x03C0, 0x01C0, 0x01E0, 0x00E0, 0x00E0, 0x0070, 0x0070, 0x0070, 0x0070, 0x0038, 0x0038, 0x0038, 0x0038,
    // ;8
    0x01F8, 0x07FE, 0x07FE, 0x0F0F, 0x0E07, 0x0E07, 0x0F0F, 0x07FE, 0x03FC, 0x07FE, 0x0F0F, 0x0E07, 0x0E07, 0x0E07, 0x0F0F, 0x07FE, 0x03FC, 0x01F8,
    // ;9                       
    0x00F8, 0x03FC, 0x07FE, 0x070F, 0x0607, 0x0E07, 0x0E07, 0x0F0F, 0x0FFE, 0x0FFC, 0x0EF8, 0x0E00, 0x0E00, 0x0707, 0x078F, 0x03FE, 0x01FE, 0x00F8,
    // ;-
    // 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFF0, 0xFFF0, 0xFFF0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0FFF, 0x0FFF, 0x0FFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};
/*
// - - - - - - - - - -
// navpicen graf
unsigned int Graf128(unsigned int *dat)  // za graf meritev
{
unsigned int i,min,max,y,pp, *d;
min=0xffff;
max=0;
d=dat;
i=128;
do {
if (min>*d) min=*d;
if (max<*d) max=*d;
d++;
  } while (--i);
pp=max-min;
GrY=0;
GrX=64-30;
pdecwu(pp);  
if (pp<1) pp=1;
y=127;
i=0;
do {
plot((((long)(max-dat[i])*63)/pp), y--);
  } while (++i<128);
GrX=0;
GrY=120;
pdecwuz(max);
GrX=64-30;
pdecwu(min);  
return max-min;
}
*/
//===========================================================================
#else
// lezeca postavitev LCD
const unsigned long LFONT[];
const unsigned char CRKE5[];
const unsigned int CRKE7[];
const unsigned int SIMBOLI[];

void GRchar5(unsigned char c)
{
    u16 i,t,d,ry,ay;
    switch (c&0xff)
    {
    case 13:
     	GrX=0;
     	GrY+=8; 
        return;
    case 0x9e: // 'ž': 
        t=0x7d-32;
        break;
    case 0x9a: // 'š':
        t=0x7c-32;
        break;
    case 0xA1: t=0x9f-32; //'obrnjen klicaj
    break;
    // case 0xe8: // 'è':
    case 'è': // 'è':
    case 0xc8: // 'è':
 	t=0x7b-32;
 	break; 
    default:
        //   if (c>0xa0)
 	c&=0xff;
        t=c-32;
    }
    if (t<=0xea)
    {
  	t=t*5+5;
  	ry=GrY&7;    // siftanje znaka navzdol
  	ay=127-GrX;
  	if (GrX<128)
        {
            ay+=((GrY&0x38)<<4);  // naslov prve tocke
            i=129-GrX;
            if (i>6) i=6;
            GrX+=i;
            /*      if ((GrX)>128-5)
            {
            GrX=0;
            GrY+=8;
        } */
            while (i--)
                if (ay<1024)
                {
                    if (i)
    			d=CRKE5[--t] ^ inv;
                    else
    			d=inv;  //presledek na koncu znaka
                    //    		LCD[ay--] |= d >> ry;  // obrnjene data linije
                    LCD[ay--] |= d << ry;
                    if (ry)
    			if (ay<1024-129)
                            //    				LCD[ay+129] |= d << (8-ry); // obrnjene data linije
                            LCD[ay+129] |= d >> (8-ry);
                }
        }     
    }
}

// izpis na LCD s srednjimi znaki 
void GRchar7(unsigned char c)
{
    u16 i,t,d,ry,ay;
    c&=0xff;
    switch (c)
    {
    case 13:
        GrX=0;
        GrY+=12+2;
        break;
    default:
        t=c-32;
        if (t<=0xea)
        {
            t=t*7+7;
            ry=GrY&7;    // siftanje znaka navzdol
            ay=127-GrX;
            if (GrX<128)
            {
                ay+=((GrY&0x38)<<4);  // naslov prve tocke
                i=128-GrX;
                if (i>8) i=8;
                GrX+=i;
                /*      if ((GrX+=i)>128-7)
                {
                GrX=0;
                GrY+=12+2;
            } */
                while (i--)
                    if (ay<1024)
                    {
                        if (i)
                            d=CRKE7[--t] ^ inv;
                        else
                            d=inv;  //presledek na koncu znaka
                        //    		LCD[ay--] |= d >> (8+ry);  // obrnjene data linije
                        LCD[ay--] |= d << ry;
   			if (ay<1024-129)
                            //   			LCD[ay+129] |= d >> ry; // obrnjene data linije
                            LCD[ay+129] |= d >> (8-ry);
                        if (ry>4)
                            if (ay<1024-129-128)
                                //    				LCD[ay+129+128] |= d << (8-ry); // obrnjene data linije
    				LCD[ay+129+128] |= d >> (16-ry);
                    }
            }     
        } 
    }
}

// izpis na LCD z velikimi znaki 
void GRchar12(unsigned char c)
{
    u16 i,t,ry,ay,s;
    unsigned long d;
    switch (c)
    {
    case 32:
 	GrX+=12+2;  //presledek
        break;
    case 13:
        GrX=0;
        GrY+=18+5; 
        break;
    default:
        t=c-0x2b;
        if (t<0x10)
        {
            if (t>='0'-0x2b)
            {
  		if (t>'9'-0x2b) // :
  		{
                    t=122;
                    s=3;
  		}
  		else
  		{
                    t=(t-('0'-0x2b))*12;
                    s=12;
  		}  		
            }
            else
            {
		switch (c)
 		{
 		case '.':	t=120; s=2; break;
 		case '-':	t=133; s=11; break;
 		case '+':	t=126; s=11; break;
		default:
                    s=0;
  		}
            }
            ry=GrY&7;    // siftanje znaka navzdol
            ay=127-GrX;
            if (GrX<128)
            {
                ay+=((GrY&0x38)<<4);  // naslov prve tocke
                i=128-GrX;	  
                if (i>s) i=s;
                GrX+=i+2;
                /*      if ((GrX)>128-12)
                {
                GrX=0;
                GrY+=18+5;
            } */
                t+=i;
                while (i--)
                    if (ay<1024)
                    {
   			d=LFONT[--t];
                        //    		LCD[ay--] |= d >> (16+ry);  // obrnjene data linije
                        LCD[ay--] |= d << ry;
   			if (ay<1024-129)
                            //   			LCD[ay+129] |= d >> (8+ry); // obrnjene data linije
                            LCD[ay+129] |= d >> (8-ry);
   			if (ay<1024-129-128)
                            //   			LCD[ay+129+128] |= d >> (ry); // obrnjene data linije
                            LCD[ay+129+128] |= d >> (16-ry);
                        if (ry>6)
                            if (ay<1024-129-256)
                                //    				LCD[ay+129+256] |= d << (8-ry); // obrnjene data linije
    				LCD[ay+129+256] |= d >> (24-ry);
                    }
            }     
        }
    }
}

// - - - - -
// izris simbola 16*16
void GRsimbol(u16 t)
{
    u16 i,d,ry,ay;
    if (t<2)
    {
  	t=t*16;
  	ry=GrY&7;    // siftanje znaka navzdol
  	ay=127-GrX;
  	if (GrX<128)
        {
            ay+=((GrY&0x38)<<4);  // naslov prve tocke
            i=128-GrX;
            if (i>16) i=16;
            while (i--)
                if (ay<1024)
                {
                    d=SIMBOLI[t++] ^ inv;
                    LCD[ay--] |= d << ry;
                    if (ay<1024-129)
    			LCD[ay+129] |= d >> (8-ry);
                    if (ry>0)
    			if (ay<1024-129-128)
                            LCD[ay+129+128] |= d >> (16-ry);
                }
            GrX+=16-1;
        }
    }
}

// ;---------------
// LFONTB:  ;BATERIJA
const unsigned char LFONTB[]={
    0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF, 0x42, 0x3C
};

// ;---------------
// LFONT:
const unsigned long LFONT[]={
    // ; 0
    0x001FE0, 0x007FFC, 0x01FFFE, 0x03E01F, 0x038007, 0x038007, 0x038007, 0x038007, 0x03E01F, 0x01FFFE, 0x007FFC, 0x001FE0,
    // ; 1
    0x000000, 0x000000, 0x000000, 0x03FFFF, 0x03FFFF, 0x03FFFF, 0x00001E, 0x00003C, 0x000078, 0x000070, 0x000000, 0x000000,
    // ;2
    0x0380F8, 0x0381FE, 0x0383FE, 0x03878F, 0x038F07, 0x039E07, 0x03BC07, 0x03F807, 0x03F00F, 0x03E03E, 0x03C03C, 0x030038,
    // ;3
    0x007C78, 0x00FEFE, 0x01FEFE, 0x03C7C7, 0x038387, 0x038387, 0x038387, 0x03800F, 0x03C01E, 0x01F01E, 0x00F018, 0x003000,
    // ;4
    0x003800, 0x003800, 0x03FFFF, 0x03FFFF, 0x03FFFF, 0x00381E, 0x00387C, 0x0039F0, 0x003FE0, 0x003F80, 0x003E00, 0x003C00,
    // ;5
    0x003E00, 0x00FF87, 0x01FFC7, 0x01C1E7, 0x0380E7, 0x0380E7, 0x0380E7, 0x0380E7, 0x03C1FF, 0x01F1FF, 0x00F1FC, 0x003180,
    // ;6
    0x007C18, 0x00FE1E, 0x01FF1E, 0x03C78F, 0x038387, 0x038387, 0x038387, 0x03838F, 0x01C71E, 0x01FFFC, 0x00FFF8, 0x001FE0,
    // ;7
    0x00000F, 0x00001F, 0x00003F, 0x0000FF, 0x0003F7, 0x003FE7, 0x03FF87, 0x03FC07, 0x03C007, 0x000007, 0x000007, 0x000007,
    // ;8
    0x007C78, 0x00FEFE, 0x01FFFE, 0x03C7CF, 0x038387, 0x038387, 0x038387, 0x038387, 0x03C7CF, 0x01FFFE, 0x00FEFE, 0x007C78,
    // ;9                       
    0x001FE0, 0x007FFC, 0x00FFFE, 0x01E38E, 0x03C707, 0x038707, 0x038707, 0x038707, 0x03C78F, 0x01E3FE, 0x01E1FC, 0x0060F8,
    // ; DECIMALNA PIKA
    0x038000, 0x038000,
    // ; DVOPICJE
    0x007070, 0x007070, 0x007070,
    // ; PLUS (SIRINA 12)
    0x000000, 0x000700,
    // ; PLUS (SIRINA 9)
    0x000700, 0x000700, 0x000700, 0x00FFF8,
    // ; DSL B'000011111111111000000000
    0x00FFF8,
    // ; DSL B'000011111111111000000000
    0x00FFF8,
    // ; DSL B'000011111111111000000000
    // ; MINUS (SIRINA 12)
    0x000700, 0x000700, 0x000700,
    // ; MINUS (SIRINA 9)
    0x000700, 0x000700, 0x000700, 0x000700, 0x000700, 0x000700, 0x000700, 0x000700, 0x000700
};

// ;---------------
// TABCHAR:
// ; normalni znaki
// ;---------------
// ; h'20
const unsigned char CRKE5[]={
    // ; presledek
    0x00, 0x00, 0x00, 0x00, 0x00,
    // ; $00,$00,$5f,$00,$00 ; "!"
    0x00, 0x00, 0x5F, 0x00, 0x00,
    // ; $00,$07,$00,$07,$00 ; """
    0x00, 0x07, 0x00, 0x07, 0x00,
    // ; $14,$7f,$14,$7f,$14 ; "#"
    0x14, 0x7F, 0x14, 0x7F, 0x14,
    // ; $24,$2a,$7f,$2a,$12 ; "$"
    0x12, 0x2A, 0x7F, 0x2A, 0x24,
    // ; $63,$13,$08,$64,$63 ; "%"
    0x63, 0x64, 0x08, 0x13, 0x63,
    // ; $30,$4e,$59,$26,$50 ; "&"
    0x50, 0x22, 0x55, 0x49, 0x36,
    // ; $00,$04,$02,$01,$00 ; "'"
    0x00, 0x00, 0x03, 0x05, 0x00,
    // ; $00,$1c,$22,$41,$00 ; "("
    0x00, 0x41, 0x22, 0x1C, 0x00,
    // ; $00,$41,$22,$1c,$00 ; ")"
    0x00, 0x1C, 0x22, 0x41, 0x00,
    // ; $2a,$1c,$7f,$1c,$2a ; "*"
    0x2A, 0x1C, 0x7F, 0x1C, 0x2A,
    // ; $08,$08,$3e,$08,$08 ; "+"
    0x08, 0x08, 0x3E, 0x08, 0x08,
    // ; $00,$40,$30,$00,$00 ; ","
    0x00, 0x00, 0x30, 0x50, 0x00,
    // ; $08,$08,$08,$08,$08 ; "-"
    0x08, 0x08, 0x08, 0x08, 0x08,
    // ; $00,$60,$60,$00,$00 ; "."
    0x00, 0x00, 0x60, 0x60, 0x00,
    // ; $60,$10,$08,$04,$03 ; "/"
    0x03, 0x04, 0x08, 0x10, 0x60,
    // ; h'30
    // ; $3e,$51,$49,$45,$3e ; "0"
    0x3E, 0x45, 0x49, 0x51, 0x3E,
    // ; $00,$42,$7f,$40,$00 ; "1"
    0x00, 0x40, 0x7F, 0x42, 0x00,
    // ; $62,$51,$51,$49,$46 ; "2"
    0x46, 0x49, 0x51, 0x51, 0x62,
    // ; $22,$41,$49,$49,$36 ; "3"
    0x36, 0x49, 0x49, 0x41, 0x22,
    // ; $18,$14,$12,$7f,$10 ; "4"
    0x10, 0x7F, 0x12, 0x14, 0x18,
    // ; $27,$45,$45,$45,$3d ; "5"
    0x39, 0x45, 0x45, 0x45, 0x27,
    // ; $3c,$4a,$49,$49,$39 ; "6"
    0x30, 0x49, 0x49, 0x4A, 0x3C,
    // ; $01,$71,$09,$05,$03 ; "7"
    0x03, 0x05, 0x09, 0x71, 0x01,
    // ; $36,$49,$49,$49,$36 ; "8"
    0x36, 0x49, 0x49, 0x49, 0x36,
    // ; $46,$49,$49,$29,$1e ; "9"
    0x1E, 0x29, 0x49, 0x49, 0x06,
    // ; $00,$00,$14,$00,$00 ; dvopicje
    0x00, 0x00, 0x36, 0x36, 0x00,
    // ; $00,$40,$34,$00,$00 ; ";"
    0x00, 0x00, 0x36, 0x46, 0x00,
    // ; $08,$14,$22,$41,$41 ; "<"
    0x00, 0x41, 0x22, 0x14, 0x08,
    // ; $14,$14,$14,$14,$14 ; "="
    0x14, 0x14, 0x14, 0x14, 0x14,
    // ; $41,$41,$22,$14,$08 ; ">"
    0x08, 0x14, 0x22, 0x41, 0x00,
    // ; $02,$01,$51,$09,$06 ; "?"
    0x06, 0x09, 0x51, 0x01, 0x02,
    // ; h'40
    // ; $38,$44,$48,$30,$4c ; alfa
    0x4C, 0x30, 0x48, 0x44, 0x38,
    // ; $7e,$09,$09,$09,$7e ; "A"
    0x7E, 0x09, 0x09, 0x09, 0x7E,
    // ; $41,$7f,$49,$49,$36 ; "B"
    0x36, 0x49, 0x49, 0x49, 0x7F,
    // ; $3e,$41,$41,$41,$22 ; "C"
    0x22, 0x41, 0x41, 0x41, 0x3E,
    // ; $41,$7f,$41,$41,$3e ; "D"
    0x3E, 0x41, 0x41, 0x41, 0x7F,
    // ; $7f,$49,$49,$49,$41 ; "E"
    0x41, 0x49, 0x49, 0x49, 0x7F,
    // ; $7f,$09,$09,$09,$01 ; "F"
    0x01, 0x09, 0x09, 0x09, 0x7F,
    // ; $3e,$41,$41,$49,$3a ; "G"
    0x3A, 0x49, 0x41, 0x41, 0x3E,
    // ; $7f,$08,$08,$08,$7f ; "H"
    0x7F, 0x08, 0x08, 0x08, 0x7F,
    // ; $00,$41,$7f,$41,$00 ; "I"
    0x00, 0x41, 0x7F, 0x41, 0x00,
    // ; $20,$40,$41,$3f,$01 ; "J"
    0x01, 0x3F, 0x41, 0x40, 0x20,
    // ; $7f,$08,$14,$22,$41 ; "K"
    0x41, 0x22, 0x14, 0x08, 0x7F,
    // ; $7f,$40,$40,$40,$40 ; "L"
    0x40, 0x40, 0x40, 0x40, 0x7F,
    // ; $7f,$02,$0c,$02,$7f ; "M"
    0x7F, 0x02, 0x0C, 0x02, 0x7F,
    // ; $7f,$04,$08,$10,$7f ; "N"
    0x7F, 0x10, 0x08, 0x04, 0x7F,
    // ; $3e,$41,$41,$41,$3e ; "O"
    0x3E, 0x41, 0x41, 0x41, 0x3E,
    // ; h'50
    // ; $7f,$09,$09,$09,$06 ; "P"
    0x06, 0x09, 0x09, 0x09, 0x7F,
    // ; $3e,$41,$51,$21,$5e ; "Q"
    0x5E, 0x21, 0x51, 0x41, 0x3E,
    // ; $7f,$09,$19,$29,$46 ; "R"
    0x46, 0x29, 0x19, 0x09, 0x7F,
    // ; $26,$49,$49,$49,$32 ; "S"
    0x32, 0x49, 0x49, 0x49, 0x26,
    // ; $01,$01,$7f,$01,$01 ; "T"
    0x01, 0x01, 0x7F, 0x01, 0x01,
    // ; $3f,$40,$40,$40,$3f ; "U"
    0x3F, 0x40, 0x40, 0x40, 0x3F,
    // ; $07,$18,$60,$18,$07 ; "V"
    0x07, 0x18, 0x60, 0x18, 0x07,
    // ; $7f,$20,$18,$20,$7f ; "W"
    0x3F, 0x40, 0x38, 0x40, 0x3F,
    // ; $63,$14,$08,$14,$63 ; "X"
    0x63, 0x14, 0x08, 0x14, 0x63,
    // ; $07,$08,$78,$08,$07 ; "Y"
    0x07, 0x08, 0x70, 0x08, 0x07,
    // ; $61,$51,$49,$45,$43 ; "Z"
    0x43, 0x45, 0x49, 0x51, 0x61,
    // ; $00,$7f,$41,$41,$00 ; "["
    0x00, 0x41, 0x41, 0x7F, 0x00,
    // ; $48,$7e,$49,$49,$42 ; "\"
    0x60, 0x10, 0x08, 0x04, 0x03, 	// 0x42, 0x49, 0x49, 0x7E, 0x48,
    // ; $00,$41,$41,$7f,$00 ; "]"
    0x00, 0x7F, 0x41, 0x41, 0x00,
    // ; $04,$02,$7f,$02,$04 ; "^"
    0x04, 0x02, 0x7F, 0x02, 0x04,
    // ; $08,$1c,$2a,$08,$08 ; "_"
    0x08, 0x08, 0x2A, 0x1C, 0x08,
    // ; h'60
    // ; $c0,$3e,$01,$49,$36 ; beta
    0x36, 0x49, 0x01, 0xFE, 0x00,
    // ; $20,$54,$54,$7c,$40 ; "a"
    0x78, 0x54, 0x54, 0x54, 0x20,
    // ; $7f,$48,$44,$44,$38 ; "b"
    0x38, 0x44, 0x44, 0x48, 0x7F,
    // ; $38,$44,$44,$44,$44 ; "c"
    0x44, 0x44, 0x44, 0x44, 0x38,
    // ; $38,$44,$44,$48,$7f ; "d"
    0x7F, 0x48, 0x44, 0x44, 0x38,
    // ; $38,$54,$54,$54,$08 ; "e"
    0x08, 0x54, 0x54, 0x54, 0x38,
    // ; $04,$7e,$05,$01,$02 ; "f"
    0x02, 0x01, 0x05, 0x7E, 0x04,
    // ; $18,$a4,$a4,$a4,$78 ; "g"
    0x78, 0xA4, 0xA4, 0xA4, 0x18,
    // ; $7f,$08,$04,$04,$78 ; "h"
    0x78, 0x04, 0x04, 0x08, 0x7F,
    // ; $00,$44,$7d,$40,$00 ; "i"
    0x00, 0x40, 0x7D, 0x44, 0x00,
    // ; $40,$80,$84,$7d,$00 ; "j"
    0x00, 0x7D, 0x84, 0x80, 0x40,
    // ; $7f,$20,$10,$28,$44 ; "k"
    0x00, 0x44, 0x28, 0x10, 0x7F,
    // ; $00,$01,$7f,$40,$00 ; "l"
    0x00, 0x40, 0x7F, 0x41, 0x00,
    // ; $7c,$04,$38,$04,$7c ; "m"
    0x78, 0x04, 0x38, 0x04, 0x7C,
    // ; $7c,$08,$04,$04,$78 ; "n"
    0x78, 0x04, 0x04, 0x08, 0x7C,
    // ; $38,$44,$44,$44,$38 ; "o"
    0x38, 0x44, 0x44, 0x44, 0x38,
    // ; h'70
    // ; $fc,$24,$24,$24,$18 ; "p"
    0x18, 0x24, 0x24, 0x24, 0xFC,
    // ; $18,$24,$24,$24,$fc ; "q"
    0xFC, 0x24, 0x24, 0x24, 0x18,
    // ; $7c,$08,$04,$04,$08 ; "r"
    0x08, 0x04, 0x04, 0x08, 0x7C,
    // ; $48,$54,$54,$54,$20 ; "s"
    0x20, 0x54, 0x54, 0x54, 0x48,
    // ; $04,$3f,$44,$44,$20 ; "t"
    0x20, 0x40, 0x44, 0x3F, 0x04,
    // ; $3c,$40,$40,$3c,$40 ; "u"
    0x7C, 0x20, 0x40, 0x40, 0x3C,
    // ; $0c,$30,$40,$30,$0c ; "v"
    0x0C, 0x30, 0x40, 0x30, 0x0C,
    // ; $3c,$40,$30,$40,$3c ; "w"
    0x3C, 0x40, 0x30, 0x40, 0x3C,
    // ; $44,$28,$10,$28,$44 ; "x"
    0x44, 0x28, 0x10, 0x28, 0x44,
    // ; $4c,$50,$50,$3c,$00 ; "y"
    0x00, 0x3C, 0x50, 0x50, 0x0C,
    // ; $44,$64,$54,$4c,$44 ; "z"
    0x44, 0x4C, 0x54, 0x64, 0x44,
    // ; $38,$44,$45,$45,$44 ; c'
    0x44, 0x45, 0x45, 0x44, 0x38,
    // ; $48,$54,$55,$55,$24 ; s'
    0x24, 0x55, 0x55, 0x54, 0x48,
    // ; $44,$65,$55,$4d,$44 ; z'
    0x44, 0x4D, 0x55, 0x65, 0x44,
    // ; $4e,$71,$01,$71,$4e ; ohm
    0x4E, 0x71, 0x01, 0x71, 0x4E,
    // ; $80,$7c,$40,$40,$3c ; mikro
    0x3C, 0x40, 0x40, 0x7C, 0x80,
    // ; h'80
    // ; ; 128   fi
    0x1C, 0x24, 0xF8, 0x24, 0x18,
    // ; ; 129 DB'
    0x3D, 0x41, 0x41, 0x41, 0x3D,
    // ; ; 130 DB'  L
    0x14, 0x2A, 0x2A, 0x6B, 0x00,
    // ; ; 131 DB'  C
    0x14, 0x14, 0x77, 0x14, 0x14,
    // ; ; 132 DB' trikot
    // 0xC0, 0xA0, 0x90, 0xA0, 0xC0,
    0x60, 0x50, 0x48, 0x50, 0x60,
    // ; ; 133 DB' zvezda
    // 0x80, 0x40, 0x38, 0x40, 0x80,
    0x40, 0x20, 0x1c, 0x20, 0x40,
    // ; ; 134 DB' stopinja
    0x00, 0x00, 0x07, 0x05, 0x07,
    // ; h'87
    // ; ; CKAA DB'
    0x78, 0x15, 0x14, 0x15, 0x78,
    // ; 88 ; CKA DB'
    0x78, 0x15, 0x16, 0x14, 0x78,
    // ; 89 ; CKE DB'
    0x44, 0x55, 0x56, 0x54, 0x7C,
    // ; 8a ; CKO DB'
    0x38, 0x45, 0x46, 0x44, 0x38,
    // ; 8b ; CKN DB'
    0x7D, 0x22, 0x11, 0x0A, 0x7C,
    // ; 8c ; 'é ; CKEE DB'
    0x18, 0x55, 0x56, 0x54, 0x38,
    // ; CDC EQU H'8D'
    0x20, 0x45, 0x45, 0x45, 0x38,
    // ; CDS EQU H'8E'
    0x20, 0x55, 0x55, 0x55, 0x48,
    // ; CDZ EQU H'8F'
    0x44, 0x4D, 0x55, 0x65, 0x44,
    // ; ; H'90 (H'F6)'  suma
    0x41, 0x41, 0x49, 0x55, 0x63,
    // ; 91 ; $08,$3c,$04,$3c,$04 ; pi
    0x04, 0x3C, 0x04, 0x3C, 0x08,
    // ; - - -
    // ; 92  ;ae
    0x58, 0x54, 0x7C, 0x54, 0x20,
    // ; 93  ;AE
    0x49, 0x49, 0x7F, 0x09, 0x7E,
    // ; 94  ;a
    0x78, 0x57, 0x55, 0x57, 0x20,
    // ; 95  ;A 
    0x78, 0x17, 0x15, 0x17, 0x78,
    // - - 
    // ; 96  ; ":a" ä
    0x78, 0x55, 0x54, 0x55, 0x20,
    // ; 97  ; "^e"
    0x18, 0x56, 0x55, 0x56, 0x38,
    // ; 98  ; ":u" ü
    0x7d, 0x20, 0x40, 0x40, 0x3d,
    // ; - - -
    // ; 99  ; 'á'
    0x78, 0x55, 0x56, 0x54, 0x20,
    // ; 9a  ; 'í'
    0x00, 0x41, 0x7e, 0x44, 0x00,
    // ; 9b  ; 'ú'
    0x7c, 0x21, 0x42, 0x40, 0x3c,
    // ; 9c  ; 'ù'
    0x7c, 0x22, 0x45, 0x42, 0x3c,
    // ; 9d  ; "/o"
    0x38, 0x4c, 0x54, 0x64, 0x38,
    // ; 9e  ; "`a
    0x78, 0x54, 0x56, 0x55, 0x20,
    // ; 9f  ; obrnjen klicaj
    0x00, 0x00, 0xFa, 0x00, 0x00,
    // ; - - -
    // TABCHAR2:
    // ; ruski znaki
    // ; a0
};

// ;---------------
// MFONT:
// ;---------------
const unsigned int CRKE7[]={
    // ; H'20  
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    // ; H'21 !
    0x0000, 0x0000, 0x0000, 0x037F, 0x037F, 0x0000, 0x0000,
    // ; H'22 "
    0x0000, 0x0007, 0x0007, 0x0000, 0x0007, 0x0007, 0x0000,
    // ; H'23 #
    0x00CC, 0x03FF, 0x03FF, 0x00CC, 0x03FF, 0x03FF, 0x00CC,
    // ; H'24 $
    0x00E4, 0x01F6, 0x01B6, 0x03FF, 0x01B6, 0x01BE, 0x009C,
    // ; H'25 %
    0x0383, 0x0287, 0x039E, 0x0078, 0x01E7, 0x0385, 0x0307,
    // ; H'26 &
    0x0260, 0x03C0, 0x018E, 0x03FF, 0x0373, 0x03FF, 0x01DE,
    // ; H'27 '
    0x0000, 0x0000, 0x000F, 0x001F, 0x003B, 0x0033, 0x0000,
    // ; H'28 (
    0x0000, 0x0201, 0x0303, 0x0186, 0x00FC, 0x0078, 0x0000,
    // ; H'29 )
    0x0000, 0x0078, 0x00FC, 0x0186, 0x0303, 0x0201, 0x0000,
    // ; H'2a *
    0x00CC, 0x0078, 0x0030, 0x01FE, 0x0030, 0x0078, 0x00CC,
    // ; H'2b +
    0x0000, 0x0030, 0x0030, 0x01FE, 0x01FE, 0x0030, 0x0030,
    // ; H'2c ,
    0x0000, 0x0000, 0x0000, 0x0300, 0x0700, 0x0400, 0x0000,
    // ; H'2d -
    0x0000, 0x0030, 0x0030, 0x0030, 0x0030, 0x0030, 0x0030,
    // ; H'2e .
    0x0000, 0x0000, 0x0000, 0x0300, 0x0300, 0x0000, 0x0000,
    // ; H'2f /
    0x0003, 0x0007, 0x001E, 0x0078, 0x01E0, 0x0380, 0x0300,
    // ; H'30 0
    0x00FC, 0x01FE, 0x0387, 0x0303, 0x0387, 0x01FE, 0x00FC,
    // ; H'31 1
    0x0000, 0x0000, 0x03FF, 0x03FF, 0x0003, 0x0006, 0x0004,
    // ; H'32 2
    0x030E, 0x031F, 0x033B, 0x0373, 0x03E3, 0x03C7, 0x0386,
    // ; H'33 3
    0x01EE, 0x03FF, 0x0333, 0x0333, 0x0303, 0x0387, 0x0186,
    // ; H'34 4
    0x00C0, 0x03FF, 0x03FF, 0x00CE, 0x00FC, 0x00F0, 0x00C0,
    // ; H'35 5
    // 0x01E3, 0x03F3, 0x0333, 0x0333, 0x0333, 0x03BF, 0x01BF,
    0x01F3, 0x03FB, 0x031B, 0x031B, 0x031B, 0x039F, 0x019F,
    // ; H'36 6
    0x01E2, 0x03F3, 0x0333, 0x0333, 0x0333, 0x03FF, 0x01FE,
    // ; H'37 7
    0x0003, 0x000F, 0x003F, 0x00F3, 0x03C3, 0x0303, 0x0003,
    // ; H'38 8
    0x01EE, 0x03FF, 0x0333, 0x0333, 0x0333, 0x03FF, 0x01EE,
    // ; H'39 9
    0x01FE, 0x03FF, 0x0333, 0x0333, 0x0333, 0x033F, 0x011E,
    // ; H'3a :
    0x0000, 0x0000, 0x0000, 0x030C, 0x030C, 0x0000, 0x0000,
    // ; H'3b ;
    0x0000, 0x0000, 0x0000, 0x030C, 0x070C, 0x0400, 0x0000,
    // ; H'3c <
    0x0000, 0x0303, 0x0186, 0x00CC, 0x0078, 0x0030, 0x0000,
    // ; H'3d =
    0x0000, 0x00CC, 0x00CC, 0x00CC, 0x00CC, 0x00CC, 0x0000,
    // ; H'3e >
    0x0000, 0x0030, 0x0078, 0x00CC, 0x0186, 0x0303, 0x0000,
    // ; H'3f ?
    0x0000, 0x000E, 0x001F, 0x0373, 0x0363, 0x0003, 0x0006,
    // ; H'40 @
    0x01FE, 0x03FF, 0x0303, 0x03F3, 0x0333, 0x03F3, 0x01E2,
    // ; H'41 A
    0x03FC, 0x03FE, 0x00C7, 0x00C3, 0x00C7, 0x03FE, 0x03FC,
    // ; H'42 B
    0x01DE, 0x03FF, 0x0333, 0x0333, 0x0333, 0x03FF, 0x03FF,
    // ; H'43 C
    0x0186, 0x0387, 0x0303, 0x0303, 0x0387, 0x01FE, 0x00FC,
    // ; H'44 D
    0x00FC, 0x01FE, 0x0387, 0x0303, 0x0303, 0x03FF, 0x03FF,
    // ; H'45 E
    0x0303, 0x0333, 0x0333, 0x0333, 0x0333, 0x03FF, 0x03FF,
    // ; H'46 F
    0x0003, 0x0033, 0x0033, 0x0033, 0x0033, 0x03FF, 0x03FF,
    // ; H'47 G
    0x01E6, 0x03E7, 0x0363, 0x0303, 0x0387, 0x01FE, 0x00FC,
    // ; H'48 H
    0x03FF, 0x03FF, 0x0030, 0x0030, 0x0030, 0x03FF, 0x03FF,
    // ; H'49 I
    0x0000, 0x0000, 0x0000, 0x03FF, 0x03FF, 0x0000, 0x0000,
    // ; H'4a J
    0x01FF, 0x03FF, 0x0300, 0x0300, 0x0300, 0x0380, 0x0180,
    // ; H'4b K
    0x0201, 0x0303, 0x0186, 0x00CC, 0x0078, 0x03FF, 0x03FF,
    // ; H'4c L
    0x0300, 0x0300, 0x0300, 0x0300, 0x0300, 0x03FF, 0x03FF,
    // ; H'4d M
    0x03FF, 0x03FF, 0x001C, 0x0038, 0x001C, 0x03FF, 0x03FF,
    // ; H'4e N
    0x03FF, 0x03FF, 0x0070, 0x0038, 0x001C, 0x03FF, 0x03FF,
    // ; H'4f O
    0x01FE, 0x03FF, 0x0303, 0x0303, 0x0303, 0x03FF, 0x01FE,
    // ; H'50 P
    0x001E, 0x003F, 0x0033, 0x0033, 0x0033, 0x03FF, 0x03FF,
    // ; H'51 Q
    0x02FC, 0x03FE, 0x0187, 0x0383, 0x0387, 0x01FE, 0x00FC,
    // ; H'52 R
    0x021C, 0x033E, 0x01F3, 0x00F3, 0x0073, 0x03FF, 0x03FF,
    // ; H'53 S
    0x01E6, 0x03F7, 0x0333, 0x0333, 0x0333, 0x03BF, 0x019E,
    // ; H'54 T
    0x0000, 0x0003, 0x0003, 0x03FF, 0x03FF, 0x0003, 0x0003,
    // ; H'55 U
    0x01FF, 0x03FF, 0x0300, 0x0300, 0x0300, 0x03FF, 0x01FF,
    // ; H'56 V
    0x003F, 0x00FF, 0x01E0, 0x0380, 0x01E0, 0x00FF, 0x003F,
    // ; H'57 W
    0x00FF, 0x01FF, 0x0380, 0x01FC, 0x0380, 0x01FF, 0x00FF,
    // ; H'58 X
    0x0303, 0x0387, 0x00FC, 0x0078, 0x00FC, 0x0387, 0x0303,
    // ; H'59 Y
    0x0000, 0x0007, 0x001F, 0x03F8, 0x03F8, 0x001F, 0x0007,
    // ; H'5a Z
    0x0307, 0x030F, 0x031B, 0x0333, 0x0363, 0x03C3, 0x0383,
    // ; H'5b [
    0x01FE, 0x03FF, 0x0303, 0x0303, 0x0303, 0x03FF, 0x01FE,
    // ; H'5c 
    0x0000, 0x03FF, 0x03FF, 0x0003, 0x0006, 0x0004, 0x0000,
    // ; H'5d ]
    0x030E, 0x031F, 0x033B, 0x0373, 0x03E3, 0x03C7, 0x0386,
    // ; H'5e ^
    0x01EE, 0x03FF, 0x0333, 0x0333, 0x0303, 0x0387, 0x0186,
    // ; H'5f _
    0x00C0, 0x03FF, 0x03FF, 0x00CE, 0x00FC, 0x00F0, 0x00C0,
    // ; H'60 `
    0x0000, 0x0000, 0x0004, 0x0006, 0x0003, 0x0001, 0x0000,
    // ; H'61 a
    0x03F8, 0x03FC, 0x036C, 0x036C, 0x036C, 0x03E8, 0x01C0,
    // ; H'62 b
    0x01F8, 0x03FC, 0x030C, 0x030C, 0x030C, 0x03FF, 0x03FF,
    // ; H'63 c
    0x0198, 0x039C, 0x030C, 0x030C, 0x030C, 0x03FC, 0x01F8,
    // ; H'64 d
    0x03FF, 0x03FF, 0x030C, 0x030C, 0x030C, 0x03FC, 0x01F8,
    // ; H'65 e
    0x0178, 0x037C, 0x036C, 0x036C, 0x036C, 0x03FC, 0x01F8,
    // ; H'66 f
    0x0003, 0x001B, 0x001B, 0x03FF, 0x03FE, 0x0018, 0x0018,
    // ; H'67 g
    0x07FC, 0x0FFC, 0x0D8C, 0x0D8C, 0x0D8C, 0x0DFC, 0x04F8,
    // ; H'68 h
    0x03F8, 0x03FC, 0x000C, 0x000C, 0x000C, 0x03FF, 0x03FF,
    // ; H'69 i
    0x0000, 0x0000, 0x0300, 0x03FB, 0x03FB, 0x0318, 0x0000,
    // ; H'6a j
    0x0000, 0x07FB, 0x0FFB, 0x0C18, 0x0C00, 0x0E00, 0x0600,
    // ; H'6b k
    0x0204, 0x030C, 0x0198, 0x00F0, 0x0060, 0x03FF, 0x03FF,
    // ; H'6c l
    0x0000, 0x0000, 0x0300, 0x03FF, 0x03FF, 0x0303, 0x0000,
    // ; H'6d m
    0x03F8, 0x03FC, 0x000C, 0x00FC, 0x000C, 0x03FC, 0x03FC,
    // ; H'6e n
    0x03F8, 0x03FC, 0x000C, 0x000C, 0x000C, 0x03FC, 0x03FC,
    // ; H'6f o
    0x01F8, 0x03FC, 0x030C, 0x030C, 0x030C, 0x03FC, 0x01F8,
    // ; H'70 p
    0x01F8, 0x03FC, 0x030C, 0x030C, 0x030C, 0x0FFC, 0x0FFC,
    // ; H'71 q
    0x0FFC, 0x0FFC, 0x030C, 0x030C, 0x030C, 0x03FC, 0x01F8,
    // ; H'72 r
    0x0000, 0x000C, 0x000C, 0x0018, 0x03FC, 0x03FC, 0x0000,
    // ; H'73 s
    0x0188, 0x03CC, 0x036C, 0x036C, 0x032C, 0x033C, 0x0118,
    // ; H'74 t
    0x0000, 0x030C, 0x030C, 0x03FE, 0x01FE, 0x000C, 0x000C,
    // ; H'75 u
    0x03FC, 0x03FC, 0x0300, 0x0300, 0x0300, 0x03FC, 0x01FC,
    // ; H'76 v
    0x001C, 0x007C, 0x01E0, 0x0380, 0x01E0, 0x007C, 0x001C,
    // ; H'77 w
    0x00FC, 0x01FC, 0x0380, 0x01F0, 0x0380, 0x01FC, 0x00FC,
    // ; H'78 x
    0x0204, 0x030C, 0x01F8, 0x00F0, 0x01F8, 0x030C, 0x0204,
    // ; H'79 y
    0x001C, 0x00FC, 0x07E0, 0x0F80, 0x08E0, 0x007C, 0x001C,
    // ; H'7a z
    0x030C, 0x031C, 0x033C, 0x036C, 0x03CC, 0x038C, 0x030C,
    // ; ZA IZPIS PREDZNAKA SREDNJE VELIKOSTI OB VELIKIH CIFRAH
    // ; H'7b { == ; H'2b +
    0x0000, 0x0030, 0x0030, 0x01FE, 0x01FE, 0x0030, 0x0030,
    // ; H'7c | == ; H'2c ,
    0x0000, 0x0000, 0x0000, 0x0300, 0x0700, 0x0400, 0x0000,
    // ; H'7d } == ; H'2d -
    0x0000, 0x0030, 0x0030, 0x0030, 0x0030, 0x0030, 0x0030,
    // ; H'7e ~
    0x0038, 0x0044, 0x0082, 0x0082, 0x0082, 0x0044, 0x0038,
    // ; H'7f 
    0x0038, 0x007C, 0x00FE, 0x00FE, 0x00FE, 0x007C, 0x0038,
    // ; H'80 €
    0x0000, 0x00F0, 0x01F8, 0x0198, 0x0FF8, 0x0FF0, 0x0180,
    // ; H'81 
    0x01FB, 0x03FB, 0x0303, 0x0303, 0x0303, 0x03FB, 0x01FB,
    // ; H'82 ‚
    0x00CC, 0x01FE, 0x01B6, 0x01B6, 0x01B6, 0x03B7, 0x03B7,
    // ; H'83 ƒ
    0x0000, 0x00D8, 0x00D8, 0x07DF, 0x07DF, 0x00D8, 0x00D8,
    // ; H'84 „
    0x0380, 0x03E0, 0x0370, 0x0338, 0x0370, 0x03E0, 0x0380,
    // ; H'85 …
    0x0000, 0x0380, 0x01C0, 0x00F8, 0x00F8, 0x01C0, 0x0380,
    // ; H'86 †
    0x0000, 0x000E, 0x001F, 0x001B, 0x001F, 0x000E, 0x0000,
    // ; H'87 ‡
    0x03F3, 0x03FB, 0x019C, 0x018C, 0x019C, 0x03FB, 0x03F3,
    // ; H'88 ˆ
    0x03F0, 0x03F8, 0x019D, 0x018F, 0x019E, 0x03F8, 0x03F0,
    // ; H'89 ‰
    0x030C, 0x030C, 0x036D, 0x036F, 0x036E, 0x03FC, 0x03FC,
    // ; H'8a Š
    0x01F8, 0x03FC, 0x030D, 0x030F, 0x030E, 0x03FC, 0x01F8,
    // ; H'8b ‹
    0x03FD, 0x03FF, 0x01C2, 0x0070, 0x0039, 0x03FF, 0x03FE,
    // ; H'8c Œ
    0x0178, 0x037C, 0x036D, 0x036F, 0x036E, 0x03FC, 0x01F8,
    // ; H'8d 
    0x0198, 0x039D, 0x030F, 0x030E, 0x030F, 0x03FD, 0x01F8,
    // ; H'8e Ž
    0x01C8, 0x03ED, 0x036F, 0x036E, 0x036F, 0x037D, 0x0138,
    // ; H'8f 
    0x030C, 0x031D, 0x033F, 0x036E, 0x03CF, 0x038D, 0x030C,
    // ; H'90 
    0x0303, 0x0313, 0x033B, 0x037F, 0x03EF, 0x03C7, 0x0383,
    // ; H'91
    0x0000, 0x0008, 0x0078, 0x0008, 0x0078, 0x0010, 0x0000,
    // ; - - -
    // ; 92  ;ae
    0x0178, 0x037C, 0x036C, 0x03FC, 0x036C, 0x03E8, 0x01C0,
    // ; 93  ;AE
    0x0333, 0x0333, 0x03FF, 0x03FF, 0x00CF, 0x03FC, 0x03F0,
    // ; 94  ;a
    0x03F8, 0x03FC, 0x036F, 0x036D, 0x036F, 0x03E8, 0x01C0,
    // ; 95  ;A 
    0x03F0, 0x03F8, 0x019F, 0x018D, 0x019F, 0x03F8, 0x03F0,
    // ; 96  ; "pp"
    0x18, 0x24, 0xFC, 0, 0x18, 0x24, 0xFC,
    // ; 97  ; "ok kljukica"
    0x07, 0x0e, 0x1c, 0x38, 0x70, 0x78, 0x1c,
    // ; 98  ; "x"
    0x63, 0x77, 0x3e, 0x1c, 0x3e, 0x77, 0x63,
    // ; 99  ; "kvadratek"
    0xef, 0x41, 0x41, 0x41, 0x41, 0x41, 0xef,
    // ; 9a  ; "prazen krogec"
    0x1c, 0x22, 0x41, 0x41, 0x41, 0x22, 0x1c,
    // ; 9b  ; "poln krogec"
    0x1c, 0x22, 0x5d, 0x5d, 0x5d, 0x22, 0x1c,
    // ; 9c  ; 	// kljucacnica
    // 0xf8, 0xfe, 0xf9, 0xc9, 0xf9, 0xfe, 0xf8,
    0x78, 0x7e, 0x79, 0x49, 0x79, 0x7e, 0x78,
    // ; 9d  ; 	// prikljucitev
    0x00, 0x15, 0x56, 0x7c, 0x56, 0x15, 0x00,
    // ; 9e  ; 	// baterija
    0x3c, 0x24, 0x24, 0x24, 0x24, 0x3c, 0x18,
    // ; 9f  ; 	// napajanje
    0x08, 0x1c, 0x22, 0x22, 0x3e, 0x14, 0x14,
    // ; - - -
    // ; ruski znaki
    // ; a0
};
// ;---------------
const unsigned int SIMBOLI[]={
    0x0008,0x0014,0x0022,0x0041, 0x0077,0x0014,0x0014,0x0014,
    0x0014,0x001c,0x0000,0x0000, 0x0000,0x0000,0x0000,0x0000,
    0x001c,0x0014,0x0014,0x0014, 0x0014,0x0077,0x0041,0x0022,
    0x0014,0x0008,0x0000,0x0000, 0x0000,0x0000,0x0000,0x0000,
};

#endif
//===========================================================================
#if 0
// ******************************************
// MODBUS komunikacija
/* Table of CRC values for high-order byte */
const unsigned char auchCRCHi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
} ; 
/* Table of CRC values for low-order byte */
const char auchCRCLo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 
    0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 
    0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 
    0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4, 
    0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 
    0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 
    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 
    0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 
    0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 
    0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 
    0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 
    0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 
    0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 
    0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 
    0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 
    0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 
    0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 
    0x43, 0x83, 0x41, 0x81, 0x80, 0x40
} ;

unsigned int CRC16byte(unsigned char b,unsigned int crc)
{
    unsigned char uchCRCLo ; /* high CRC byte initialized */
    unsigned char uchCRCHi ; /* low CRC byte initialized  */
    unsigned uIndex ;        /* will index into CRC lookup*/
    /* table  */
    uchCRCLo = crc & 0xFF;
    uchCRCHi = crc >> 8;
    uIndex = uchCRCHi ^ b;
    uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;
    uchCRCLo = auchCRCLo[uIndex] ;
    return ((uchCRCHi << 8) | uchCRCLo) ;
}

unsigned int CRC16(unsigned char *puchMsg,unsigned int usDataLen)
//unsigned int CRC16(unsigned int *buf,unsigned int usDataLen)
{
    unsigned char uchCRCHi = 0xFF ; /* high CRC byte initialized */
    unsigned char uchCRCLo = 0xFF ; /* low CRC byte initialized  */
    unsigned uIndex ;               /* will index into CRC lookup*/
    /* table  */
    //	unsigned int i;
    //	i=0;
    while (usDataLen--)             /* pass through message buffer   */
    {
        uIndex = uchCRCHi ^ *puchMsg++ ;       /* calculate the CRC  */
        //		uIndex = uchCRCHi ^ izniz(buf,i++);
        uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;
        uchCRCLo = auchCRCLo[uIndex] ;
    }
    return ((uchCRCHi << 8) | uchCRCLo) ;
}

unsigned int CRCput(unsigned char b,unsigned int crc)
{
    SendRS1(b);  // izpis znaka na PC RS232
    return CRC16byte(b,crc);
}
#endif
//===========================================================================
/*
void LCDtoRS(int tipka)
{
unsigned int crc,i,j;

crc=CRCput('!',0xffff);
crc=CRCput('K',crc);
crc=CRCput(tipka,crc);
for (j=0;j<1024;j+=128)
for (i=127;i<128;i--)
crc=CRCput((LCD[j+i]),crc);
SendRS0(crc>>8);  // izpis znaka na PC RS232
SendRS0(crc&0xff);  // izpis znaka na PC RS232
}
*/
void pdecw4(int v)
{
    prdec(v,0xa004);
}

//===========================================================================
const unsigned char TcharMB[]={'>','<','-','+',13,'S'};  

unsigned int LCDizb;

//===========================================================================
void pniz(const unsigned char *buf)
{
    unsigned char c;	
    
    while ((c=*buf++)!=0)
        putchar(c);
}

unsigned int hexcif(int n)
{
    if (n<10)
  	return('0'+n);
    else
        if (n<16+20)  //za vse crke abecede
            return('A'-10+n);
        else
            return (' ');
}
void putcif(int n)
{
    putchar(hexcif(n));
}
void putcr(void)
{
    putchar(13);
}
void putsp(void)
{
    putchar(' ');
}
void putpik(void)
{
    putchar('.');
}

void phex(unsigned int v)
{
    putcif((v>>4)&15);
    putcif(v%16);
}

void phexz(unsigned int v)
{
    if (v>15)
  	putcif((v>>4)&15);
    else
  	putsp();
    putcif(v%16);
}

void phexw(unsigned int d)
{
    phex(d>>8);
    phex(d&0xff);
}

void pdec2u(unsigned int v)
{
    putcif(v/10);
    putcif(v%10);
}

void pdecu(unsigned int d)
{
    putcif(d/100);
    pdec2u(d%100);
}

unsigned int psignw(int d)
{
    if (d<0)
    {
  	putchar('-');
  	return -d;
    }
    else
    {
  	putchar('+');
  	return d;
    }
}
//================
const long DekL[10]={1,10,100,1000,10000,100000,1000000,10000000,100000000,1000000000};

void prdecn(unsigned long v, int n)
{
    int i,c;
    for (i=n-1; i>=0;i--)
    {
        c=v/DekL[i];
        v-=c*DekL[i];
        putchar('0'+c);
    }
}

void prdec(long v, int mode)
{
    // mode bit 15:  izpis predznaka
    //      bit 14:  izpis ' ' namesto '+' predznaka 
    //      bit 13:  izpis ' ' namesto vodilnih nicel
    //      bit 12:  izpis manjse decimalke
    //      bit 8-10: stevilo mest za decimalno piko
    //      bit 7:  izpis skrajsane neg. vrednosti, brez '+' predznaka
    //      bit 6:  brez izpisa ' ' na zacetku
    // 	bit 0-3: stevilo dekadnih mest 1-9
    unsigned long u,c;
    int nt,nd,p;
    u=v;
    if (mode<0)
    {
        if (v<0)
        {
            u=-v;
            p='-';
            if (mode&0x080)  //izpis skrajsane neg. vrednosti, brez '+' predznaka
                mode--; // skrajsane neg. vrednosti
        }
        else
        {
            if (mode&0x080)  //izpis skrajsane neg. vrednosti, brez '+' predznaka
                p=0;  // brez '+' predznaka
            else
                p='+';
        }
    }
    else
        p=0;
    if (mode&0x4000) // bit 14:  izpis ' ' namesto '+' predznaka 
        if (p=='+')
            p=' ';
    
    nd=(mode>>8)&15;  //stevilo dec. mest
    nt=mode&15; 	// stevilo mest
    if (u>=DekL[nt])
        u=DekL[nt]-1;  //omejitev vrednosti
    if (mode&0x2000) // bit 13:  izpis ' ' namesto vodilnih nicel
        while ((u<DekL[nt-1])&(nt>nd+1))
        {
            nt--;
            if ((mode&0x0040)==0) //      bit 6:  brez izpisa ' ' na zacetku
                putchar(' ');
        }
    if (p)
        putchar(p); 	//predznak ali preseldek
    if (nd)	//izpis z decimalnim mestom
    {
        c=u/DekL[nd];
        prdecn(c, nt-nd);
        u-=c*DekL[nd];
        /*        	if (mode&0x1000) // bit 12:  izpis manjse decimalke
        if (OutDev)
        {
        OutDev=0;
        GrY+=3;
    }
        putpik(); */
        putchar('.');
        nt=nd;
    }
    prdecn(u, nt);
}

void pdecwu(unsigned int v)
{
    prdec(v,0x2005);
}
void pdecwuz(unsigned int v)
{
    prdec(v,0x2045);
}
void pdecw(int v)
{
    prdec(v,0xa005);
}
//******************************************************************************

//******************************************************************************
