#ifndef __HEAD_H__
#define __HEAD_H__

//#include "io430.h"
#include <msp430.h>
#include  <stdio.h>



#define PER50 40000/8
#define simulacija 0

//===========================================================================
// 74HC573 LACH biti
#define BL 0x01
#define CSSD 0x02
#define BEEP 0x40
#define LED1 0x80
#define LED2 0x20
#define LED3 0x08
//===========================================================================
#define LACH_LE1 P8OUT|= 0x02
#define LACH_LE0 P8OUT&=~0x02

#define TkGor 0x10
#define TkLev 0x02
#define TkDes 0x08
#define TkEnt 0x04
#define TkDol 0x01
#define TkRept (TkGor+TkDol+TkLev+TkDes+TkEnt)

#define CLK 0x04
#define SI  0x08
#define CS  0x10

#define FULL  1
#define CR    2


#define BUFFER_SIZE  0x1F //velikost buffra sme biti  (2^x -1)  7,15,31,63 ..
#define STDOUT_LCD               0
#define STDOUT_LCD_NORMAL_FONT   1
#define STDOUT_LCD_LARGE_FONT    2 
#define STDOUT_RS232             3
#define STDOUT_FLASH             4
#define TO_STRING                5
#define Tmax 7

#define ZACETEK              0
#define CAKA_START           1
#define KONTROLA_KABELNA     2
#define NAPAKA       3
#define KONTROLA_LCD         4
#define SERVICE_MODE         5


#define MOTOR_1_A 0x10
#define MOTOR_1_B 0x08


#define FLASH_BLOCK_SIZE 1024
#define FLASH_BUF_SIZE 1024


//int stIzbir = 2;
enum izbireProgramov{
  PROG_FLASH,
  PROG_NORM,
};

typedef struct {
  char *ime;
  int operacije;
}izbira;

//doloci kateri tip naprave bomo testirali
extern int tipNaprave;


//#define SizRcode 0x520
//__no_init extern unsigned int Rcode[];   // prostor v RAM za FLASHUP kodo
typedef signed short s16;
typedef unsigned short u16;

typedef signed long s_byte;
typedef unsigned long u_byte;
typedef signed short s_int;
typedef unsigned short u_int;
typedef signed long s_long;
typedef unsigned long u_long;











extern unsigned int U1[];
extern unsigned int Us[];
extern unsigned int U1_max[];
extern unsigned long U1_acc[];
extern unsigned long U1_acc_sqr[];
extern unsigned long Us_pov[];
extern unsigned long Us_acc_sqr[];
extern unsigned int U1_pov;
extern unsigned int U1_TRMS;
extern unsigned int Us_TRMS;
extern unsigned int Us_POV;

extern unsigned int U1_TRMS_dis;
extern unsigned int Us_TRMS_dis;
extern unsigned int Us_POV_dis;
extern unsigned int Us_max;
extern unsigned int Us_min;
extern unsigned int Us_max_dis;
extern unsigned int Us_min_dis;

extern unsigned int U1_TRMS_avr;
extern unsigned int Us_TRMS_avr;
extern unsigned int Us_POV_avr;
extern signed char draw_U1_x;
extern signed char draw_U1_y;


extern char gain;
extern char auto_gain;
extern unsigned char ojacanje[];
extern char gain_change;
extern char cnt_avr;
extern unsigned int U_konst;
extern unsigned int AD_period;

extern unsigned int TRMS_int;
extern unsigned char blok;
extern unsigned char mes_samp_cnt;
extern unsigned char blok_preracun;
extern unsigned char preracun_vzorcev;
extern unsigned char cur_dis;

extern unsigned int pnt_write_flash;
extern unsigned int flash_page;
extern unsigned int cnt_wr_fl_buf;
extern unsigned char flash_buf[];

extern unsigned int rec_flash_page;
extern unsigned char send_data_TX_finish;
extern unsigned char* pnt_send_data_TX_buf;
extern unsigned char call_back_send_data_TX;
extern unsigned int send_data_TX_len;
extern unsigned int send_data_TX_cnt;
extern unsigned char send_data_TX_finish;

extern unsigned char flash_write_on;
extern unsigned char blinking;
extern unsigned char meni_set_state;
extern char *to_str_pnt;
extern char label_cnt;
extern char st_resetov;
extern char verzija;
extern unsigned int pov;
extern int log[5000];
extern int display_state;
extern char key_pressed;
extern unsigned char P4LATCH2;
extern unsigned char kontrola;
extern unsigned char zakasnitev_ser_mode;
extern unsigned char zakasnitev_testa;

#if simulacija_odpade 
extern unsigned int AD_faza1[];
extern unsigned int sinus[];

#endif

//extern unsigned int Rcode[];


/*
//grfika.c
void LCD_init(void);
void LCD_cmd(char d);
void LCD_write(char d);
char LCD_read(int rs);
void LCD_sendC(void);
void LCD_fill(char d);
void IzpisLCD(void);
void GRchar5(char c);
void plot(s16 x, s16 y);
void line(s16 x, s16 y);
void liner(s16 x, s16 y);
void hline(int y);
void boxr(s16 dx, s16 dy);
void Hbar(s16 dx, u16 dy, s16 b);
void PrNniz(s16 n, const char *niz);
void pniz(const char *buf);
int putchar(int a);
void prdecn(unsigned long v, int n);
void prdec(long v, int mode);
void GRchar5(char c);
// izpis na LCD s srednjimi znaki 
void GRchar7(char c);
void GRchar12(char c);
*/

//util.c
void Init(void);
void BeriKey(void);
void LE573set(void);
void LE573hold(void);
int TipkaVhod(void);
//char KGet(void);
void izpis(void);
unsigned int str_cmp(unsigned char *cmp,unsigned char *str);
unsigned int str_chr(unsigned int start, unsigned char ch, unsigned char *str,unsigned int max_str );


char write_out_buf(char c);
char read_out_buf(char* c);
char write_in_buf(char c);
char read_in_buf(char* c);

//anze.c
void sahovnica(void);
void cursor(void);
void potek_kontrole(void);

void serial(void);
void out_com(void);
void izracun(void);
void sahovnica_inverzno(void);
void pas(int i);
void zgoraj_belo();
void zgoraj_crno();
void clear();
void display_test();
int prev_kable_2(void);
//rs232

void RS232Init (void); // init vars and
// set up hardware (baud rate ...)
unsigned char ucRS232RXBufferCount (void); // count of chars in
// receive buffer
char cRS232GetChar (void); // read char from receive buffer
void RS232TXChar (char cByte); // write a char to send buffer
// (and send it)
extern void(*rec_state)(void);


void rec_PARSE_COMM(void);
void rec_READ_FLASH_INI(void);
void send_data_TX(void);
void send_data_TX_ini(void);
void rec_READ_FLASH(void);




//ad.c
void Init_AD(void);
void obdelaj_podatke(void);
void menu(unsigned char display);
void set_gain(void);
void send_bit(char buf);

// ASM rutine
unsigned int CKOREN(unsigned long v);
int CATAN2(int y, int x);
int CCOS(int kot);
long CSCOS(int kot); // hi=sin lo=cos

//menu.c
void meni_large_numbers(void);
void meni_U1(void);
void meni_U2(void);
void meni_U3(void);
void meni_ref(void);
void meni_vector(void);
void meni_settings(void);
void ini_meni_max_min(void);
void meni_max_min(void);


//main
void izpis_to_RAM_LCD(void);

#endif