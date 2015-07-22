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




#define MOTOR_1_A 0x10
#define MOTOR_1_B 0x08


#define FLASH_BLOCK_SIZE 1024
#define FLASH_BUF_SIZE 1024





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




//util.c
void Init(void);
void BeriKey(void);
int TipkaVhod(void);


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
void clear_2();
void clear();
void clearArea(int start, int stop);
void clearLine(int start, int end);
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




//main
void switchApps();
void initApps();
const char *getVersion();

#endif









