/* TIMERB v1.20 */

#ifndef __TIMERB_H__
#define __TIMERB_H__

#include "scheduler.h"
//#include "helper.h"

// ---------------------------------------------------------------------------
// stevilo tierjev (max 16 zaradi integer maske!)
#define MaxTim 16
//const unsigned int BitMaskW[16]={1,2,4,8,0x10,0x20,0x40,0x80,0x100,0x200,0x400,0x800,0x1000,0x2000,0x4000,0x8000};

void timerb_init(void);   // inicializacija timerB

// t_appID index 1..16
// mili_sec 1..16000) interval 1/1024s
void timer_wait(unsigned int t_appID, unsigned int mili_sec);
void timer_wait_off(unsigned int t_appID);
void s_timer_wait(unsigned int t_appID, unsigned int sec);
void s_timer_wait_off(unsigned int t_appID);

extern unsigned int TimEnSec;  // aktivni sekundni intervali
extern unsigned int TrigSec;   // vsako sekundo se vpise TimEnSec, aplikacije brisejo pripadajoce bite
extern unsigned long RTCabsMin; // število minut od nedelje, 31.12.2000
extern unsigned int SecStev;   // stevec sekund
// vklop aktivnih sekundnih intervalov
inline void TimerSec(unsigned int app_flg)
{
  TimEnSec |= app_flg;
  TrigSec &=~app_flg;   // vsako sekundo se vpise TimEnSec, aplikacije brisejo pripadajoce bite
}
inline void TimerSec_off(unsigned int app_flg)
{
  TimEnSec &=~app_flg;
}

extern unsigned int TimEnMin;  // aktivni minutni intervali
extern unsigned int TrigMin;   // vsako minuto se vpise TimEnMin, aplikacije brisejo pripadajoce bite
// vklop aktivnih minutnih intervalov
inline void TimerMin(unsigned int app_flg)
{
  TimEnMin |= app_flg;
  TrigMin &=~app_flg;   // vsako minuto se vpise TimEnMin, aplikacije brisejo pripadajoce bite
}
inline void TimerMin_off(unsigned int app_flg)
{
  TimEnMin &=~app_flg;
}
// ---------------------------------------------------------------------------
// definicije za RTC
//__no_init unsigned char RTC[10];   // kontrol, sekunde, minute, ure, dnevi, mesci, leta, dan v tednu, abs.dan
extern volatile unsigned char RTC[10];

// kontrolni register RTCctr
#define RTCctr RTC[0]
#define STauto 1
#define STbit 2
#define DateCh 4
#define TimeCh 8
//    bit 0: dovoljen avtomatski preklop poletni/zimski cas
//    bit 1: stanje za poletnbi cas
#define RTCsec RTC[1]
#define RTCmin RTC[2]
#define RTChour RTC[3]
#define RTCday RTC[4]
#define RTCmonth RTC[5]
#define RTCyear RTC[6]
// dan v tednu 0..6 (0 = nedelja)
#define RTCdvt RTC[7]
// integer dostop ure (hi), minute (lo)
#define RTCtime ((unsigned int *)&RTC[2])[0]
// integer dostop mesca (hi), dneva (lo)
#define RTCdate ((unsigned int *)&RTC[4])[0]
// integer stevilo dni od nedelje, 31.12.2000
#define RTCabsDay ((unsigned int *)&RTC[8])[0]
// RTC rutine
unsigned long PackTime(unsigned char *RTCmb); 	// pretvorba casa v binarni format v sekundah
unsigned long UnpackTime(unsigned long t); 	// pretvorba casa iz binarnega formata
void CopyLong3Char(unsigned long t, unsigned char *RTCmb);
int AbsDanC(unsigned char *RTCmb);     // izracun stevila dni od nedelje, 31.12.2000
unsigned int Abs2binDate(int d);    // izracun datuma iz stevila dni od nedelje, 31.12.2000
long AbsMinC(unsigned char *RTCmb); // izraèun število minut od nedelje, 31.12.2000

void clock_offset(signed int sec_off);   // premik èasa v sekundah
int TestST(int set);  // test za poletni cas

void UpdateDST(char d);     // nastavi avtomatski prekolp poletni/zimski cas (STauto)
void RTC_date_set(unsigned int date, unsigned char year);   // nastavitev datuma dan (0:7), mesec (8:15)
void RTC_time_set(unsigned int time, unsigned char sec);   // nastavitev casa minute (0:7), ure (8:15)
void RTC_clock_set(unsigned char *RTCmb);   // nastavitev ure in datuma
void GSM_clock_sync(unsigned char *RTCmb);  // nastavitev ure in datuma z opcijskim premikom na zimski cas
unsigned long UnpackDate(unsigned int t);
unsigned char RTC_ckeck(void);
// ---------------------------------------------------------------------------


/* set timer CCRx trigger in ms */
#define _timer1_ms(_t)  TBCCR1 = TBR + ((unsigned int)_t << 2)
#define _timer2_ms(_t)  TBCCR2 = TBR + ((unsigned int)_t << 2)
#define _timer3_ms(_t)  TBCCR3 = TBR + ((unsigned int)_t << 2)
#define _timer4_ms(_t)  TBCCR4 = TBR + ((unsigned int)_t << 2)

/* arm timer x */
#define _timer1_arm()  TBCCTL1 = CCIE
#define _timer2_arm()  TBCCTL2 = CCIE
#define _timer3_arm()  TBCCTL3 = CCIE
#define _timer4_arm()  TBCCTL4 = CCIE

/* disarm timer x */
#define _timer1_disarm()  TBCCTL1 = 0
#define _timer2_disarm()  TBCCTL2 = 0
#define _timer3_disarm()  TBCCTL3 = 0
#define _timer4_disarm()  TBCCTL4 = 0
// ---------------------------------------------------------------------------


#endif /*__TIMER_H__*/


