#include <stdio.h>


typedef signed short s16;
typedef unsigned short u16;
#define MSP430X 0
#define MSP1X 1

#define SWver 100
#define TModName "XCOIL   verzija 1.00"
// koda za FLASHUP
#define FLTIP 1009
// dolzina imena tipske oznake za FLASHUP
#define ModelLen 5
//******************************************************************************
#define  Pos1SRBw ((unsigned char *)&PosSRB)[0]
#define  Pos1SRBr ((unsigned char *)&PosSRB)[1]
#define  Pos1STBw ((unsigned char *)&PosSTB)[0]
#define  Pos1STBr ((unsigned char *)&PosSTB)[1]
#define  Pos2SRBw ((unsigned char *)&PosSRB)[2]
#define  Pos2SRBr ((unsigned char *)&PosSRB)[3]
#define  Pos2STBw ((unsigned char *)&PosSTB)[2]
#define  Pos2STBr ((unsigned char *)&PosSTB)[3]
// USCIA0 za 1. komunikacijo RS485
#define Len1SRB 256
#define Len1STB 256
#define Ser1OutBuf ((unsigned char *)&Rcode[0x200])
// ----
// USCIA1 za 2. komunikacijo RS485
#define Len2SRB 32
#define Len2STB 32

//extern unsigned char LCDpr;  //se ne uporablja // stevilka prikaza za LCD
extern unsigned char OutDev;  // izhodna enota 0=LCD, 1=srednji fonti, 2=veliki fonti, 4=RS232 PC

#pragma data_alignment=2
extern unsigned int Rcode[];
#define LCD ((char *)&Rcode[0])
//#define LCD ((unsigned char *)&Rcode[0])
extern int GrX,GrY;   // koordinati tocke na LCD
extern unsigned int inv;        // za inverzen izpis na LCD
extern u16 SekStev;   // stevec sekund
extern s16 SetPos; 			// pozicija za nastavljanje
extern u16 LCDset; 	// koda parametra za nastavitve
extern unsigned long LCDval; 	// vrednost za nastavljanje
extern s16 selmenu[5],levmenu;  	// izbrana vrstica v meuju, nivo menuja
extern s16 PosInv; 		// pozicija zacetka inverznega izpisa

extern const unsigned char Tkode[];
extern const unsigned char Tchar[];
extern unsigned char P4LATCH;  // stanje za LATCH 74HC573
extern unsigned char krei_del;
extern char on;
extern char test_tipk_biti;
extern unsigned char KeyBuf_2;
unsigned int SwapW(unsigned int w);
void ToTX1(void);
void ToTX2(void);
int getRS1(void);
void SendRS1(unsigned char c);
int getRS2(void);
void SendRS2(unsigned char c);
int getRS(unsigned int port);
void SendRS(unsigned int port, unsigned char c);
void ToTX(unsigned int port);
void Bcopy(unsigned int *src, unsigned int *dest, unsigned int n);
//******************************************************************************
int putchar(int a);
void pniz(const unsigned char *buf);
unsigned int hexcif(int n);
void putcif(int n);
void putcr(void);
void putsp(void);
void putpik(void);
void phex(unsigned int v);
void phexz(unsigned int v);
void phexw(unsigned int d);
void pdec2u(unsigned int v);
void pdecu(unsigned int d);
unsigned int psignw(int d);
void prdecn(unsigned long v, int n);
void prdec(long v, int mode);
void pdecwu(unsigned int v);
void pdecwuz(unsigned int v);
void pdecw(int v);
//******************************************************************************
int DataFLASH_status(void);
void DFread(int PA, int BA, int n, unsigned int *BUF);
// branje podatkov iz DATA FLASH-a direktno na RS za PC
void DFreadRS(int PA, int BA, int n);
// MMC
void mcb_xmit(unsigned int d);
unsigned int mcb_rec(void);
unsigned int MMCSIN8(void);
unsigned int MMCSIN(void);
unsigned int MMC_cmd(int cmd, unsigned long par);
unsigned int MMCstatus(void);
int MMCinit(void);
int MMCidle(void);  // poslje GO_IDLE_STATE s CRC
int MMCread(unsigned long adr, unsigned char *BUF);
int MMCwrite(unsigned long adr, unsigned char *BUF);
unsigned int MMC_wrwait(int max);
int MMCreadSect(long s);
int MMCwriteSect(long s, int re);
int MMCrInfo(void);
int FindFile(unsigned char *BUF, unsigned char *name);
int LookDir(void);  // iskanje privzetih datotek in direktorijev
unsigned int RNclast(unsigned long size);
int Fopen(unsigned int cl, unsigned long size);
void Bcopy(unsigned int *src, unsigned int *dest, unsigned int n);
void Bfill(unsigned int *buf, unsigned int Len, unsigned int val);
int Fseek(unsigned long pos);   // branje sektorja za pozicijo pos
int Fget(void);
unsigned int LookFDir(unsigned int cl, unsigned int *name, unsigned long *Siz);
unsigned int ChkFATfree(void);
unsigned int MkFILE(unsigned int cl, unsigned int *name, unsigned long Len);
unsigned int MkDIR(unsigned int cl, unsigned int *name);
int Fcreate(unsigned int cl, unsigned long size); // kreiranje datoteke dolzine 'size' od clastra 'cl'
int Fdelete(unsigned int cl, unsigned int *name); // brisanje datoteke 'name' z direktorija od clastra 'cl'
int FwriteW(unsigned int w);    // zapis enega worda v kreirano datoteko
int FwriteEnd(void);            // zapis zadnjega sektorja v kreirano datoteko
//******************************************************************************
unsigned long DirTimeDate(void);
void BeriKey(void);
int  KGet(char tipka);
void LCD_init(void);
void LCD_sendC(void);
void LCD_fill(unsigned char d);
void IzpisLCD(void);
void ObdTipk(void);
void LCDtoRS(int tipka);
void PrVJniz(const char *niz);
void clear();
void clearArea(int start, int stop);
void clearLine(int start, int end);
//******************************************************************************

void LCD_fill(unsigned char d);
//===========================================================================
#if VertLCD
// pokoncna postavitev LCD
void GRchar8(unsigned char c);
void GRchar12(unsigned char c);
void GRchar18(unsigned char c);
#else
void GRchar5(unsigned char c);
void GRchar7(unsigned char c);
void GRchar12(unsigned char c);
void GRsimbol(u16 t);
#endif
void plot(s16  x, s16  y);
void line(s16  x, s16  y);
void liner(s16  x, s16  y);
void hline(s16  y);
void boxr(s16 dx, s16 dy);
void Hbar(s16 dx, u16 dy, s16 b);
void SetInv(s16 fl);
int prev_kable(void);

//===========================================================================
// 74HC573 LACH biti
#define BL 0x01
#define CSSD 0x02
#define BEEP 0x40
#define LED1 0x80
#define LED2 0x20
#define LED3 0x08
//===========================================================================
void LE573set(void);   // P4 na 574 izhode
void LE573hold(void);  // P4 za LCD in tipke
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
#define RES2 0x10
#define RS2 0x08
#define LCD_E2 0x04
#define RW 0x08
#define CS2 0x04
#define LE 0x40
#define TIPKA 0x20
#define CSFLASH 0x80

#define _set_RES          P2OUT |= RES2
#define _clear_RES        P2OUT &= ~RES2
#define _set_RS           P2OUT |= RS2
#define _clear_RS         P2OUT &= ~RS2
#define _set_LCD_E2       P2OUT |= LCD_E2
#define _clear_LCD_E2     P2OUT &= ~LCD_E2
#define _set_RW           P3OUT |= RW
#define _clear_RW         P3OUT &= ~RW
#define _set_CS           P3OUT |= CS2
#define _clear_CS         P3OUT &= ~CS2
#define _set_LE           P2OUT |= LE
#define _clear_LE         P2OUT &= ~LE
#define _set_TIPKA        P2OUT |= TIPKA
#define _clear_TIPKA      P2OUT &= ~TIPKA
#define _set_CSFLASH      P2OUT |= CSFLASH
#define _clear_CSFLASH    P2OUT &= ~CSFLASH

#define P1P2_input    P2DIR &= ~0x03;  P1DIR &= ~0xCF
#define P1P2_output    P2DIR  |= 0x03;  P1DIR |= 0xCF

//#define on_REDled      P2DIR |= 0x40; P2OUT  |= 0x40
//#define off_REDled      P2DIR &= ~0x40; P2OUT  &= ~0x40
//#define on_GREENled      P2DIR |= 0x80; P2OUT  |= 0x80
//#define off_GREENled      P2DIR &= ~0x80; P2OUT  &= ~0x80

#define on_PULSEled      P5DIR |= 0x40; P5OUT  |= 0x40          //P5.6
#define off_PULSEled      P5DIR &= ~0x40; P5OUT  &= ~0x40
#define on_ALARMled      P5DIR |= 0x80; P5OUT  |= 0x80          //P5.7
#define off_ALARMled      P5DIR &= ~0x80; P5OUT  &= ~0x80

#define BUFFER_SIZE  0x1F //velikost buffra sme biti  (2^x -1)  7,15,31,63 ..
#define STDOUT_LCD               0
#define STDOUT_LCD_NORMAL_FONT   1
#define STDOUT_LCD_LARGE_FONT    2 
#define STDOUT_RS232             3
#define STDOUT_FLASH             4
#define TO_STRING                5
#define Tmax 7


#define TkGor 0x10
#define TkLev 0x02
#define TkDes 0x08
#define TkEnt 0x04
#define TkDol 0x01
#define TkRept (TkGor+TkDol+TkLev+TkDes+TkEnt)

/* SPI za data FLASH */
/* --------------------------------
 serijski DATA FLASH
 -------------------------------- */
// stevilo blokov
//#define DFpages  4096
#define DFpages  8192
// velikost bloka (bytov)
#define DFblokS  1024


//===========================================================================

