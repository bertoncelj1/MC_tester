/* TIMERB v1.20 */

#include "msp430x24x.h"

//#include "intrinsics.h"
//#define _EINT __enable_interrupt
//#define _DINT __disable_interrupt


//#include "GSM_head.h"
//void  _cpu_on(void)
// {
// }
//void  _cpu_off_irq(void)
// {
// }

//#include "watchdog.h"

// za skrajsanje minute 60s -> 3s (ce ni definiran, je 60s)
//#define FastMin 3



#define FastMin    60

#include "scheduler.h"
#include "TIMERB.H"
#include "helper.h"



const unsigned int BitMaskW[16]={1,2,4,8,0x10,0x20,0x40,0x80,0x100,0x200,0x400,0x800,0x1000,0x2000,0x4000,0x8000};
//extern const unsigned int BitMaskW[16];
// ---------------------------------------------------------------------------

// RAM lokacije za timer za 1ms interval
unsigned int TrigTim;   // ob poteku sztevca se postavi pripadajoci bit, aplikacije brisejo pripadajoce bite
unsigned int TimEnFlg;
unsigned int TimTime[MaxTim];
signed char TimOrder[MaxTim];   // tabela zaporedja
unsigned char IorderZ,IorderK;  // zacetek in konec v tabeli zaporedja
unsigned char T_index;  // index tekocega timerja

// RAM lokacije za timer za sekundni interval
unsigned int STrigTim;  // ob poteku sztevca se postavi pripadajoci bit, aplikacije brisejo pripadajoce bite
unsigned int STimEnFlg;
unsigned int STimTime[MaxTim];
unsigned int TrigSec;   // vsako sekundo se vpise TimEnSec, aplikacije brisejo pripadajoce bite
unsigned int TimEnSec;  // aktivni sekundni intervali
unsigned int SecStev;   // stevec sekund

// RAM lokacije za RTC in minutni interval
unsigned int TrigMin;   // vsako minuto se vpise TimEnMin, aplikacije brisejo pripadajoce bite
unsigned int TimEnMin;  // aktivni minutni intervali
#pragma data_alignment=2
__no_init volatile unsigned char RTC[10];   // kontrol, sekunde, minute, ure, dnevi, mesci, leta, dan v tednu, abs.dan
unsigned long RTCabsMin; // število minut od nedelje, 31.12.2000


// ---------------------------------------------------------------------------
// inicializacija timerB
// ---------------------------------------------------------------------------
void timerb_init(void)
{
	
	TimEnFlg=0;
	IorderZ=0; IorderK=0;  // zacetek in konec v tabeli zaporedja
	STimEnFlg=0;
	TBCCTL0 = 0;  //CCIE;                           // TRCCR0 interrupt disabled
	TBCTL = TBSSEL_1 + ID_3 + MC_2 + TBCLR;         // ACLK/8, Continous up mode, CLR
	TBCCR6 = 32768U/8/2;   // 0.5s zakasnitev
	TBCCTL6 = CCIE;                           // TRCCR6 interrupt disabled
	if(!RTC_ckeck())
		//if (RTCctr>=16)  // neveljavna ura
	{
		((unsigned int *)&RTC[0])[0]=3;   // kontrol=3 in sekunde=0
		RTCtime=0;
		RTCdate=0x0210;   // 16.2. (nedelja
		((unsigned int *)&RTC[6])[0]=0x0009;   // leto (2008) in dan v tednu (0)
		RTCabsDay=AbsDanC((unsigned char *)&RTCday); // izracun stevila dni od nedelje, 31.12.2000
		//RTC_Flags.FALSE_TIME = 1;
		
		
		
	}
	RTCabsMin = AbsMinC((unsigned char *)&RTCmin); // število minut od nedelje, 31.12.2000
}

// ---------------------------------------------------------------------------
// rutine za milisekundne intervale
// ---------------------------------------------------------------------------


void timer_wait(unsigned int t_appID, unsigned int mili_sec)
{
    // t_appID index 0..15
    // mili_sec 1..16000) interval 1/1024s
    unsigned int Tmask,tim,run;
    int i;
    
    //  if (t_appID == UART_AUX_ID)
    //    _NOP();
    t_appID--;   // 1..16 -> 0..15
    if (t_appID >= MaxTim)
        return; //  neveljaven index
    if(mili_sec > 4000) return; 
    
    tim = mili_sec<<2;   // pomnozi z 4
    if (tim==0)
        return;   // nedovoljena vrednost
    Tmask=BitMaskW[t_appID];
    if (TimEnFlg & Tmask)   // ze obstaja
        timer_wait_off(t_appID+1);  // prekinitev obstojecega
    TimEnFlg^=Tmask;
    TBCCTL0 &= ~CCIE;       // TRCCR0 interrupt disabled
    run=TBR;  // tekoci cas (timer B)
    if (run!=TBR)  // moznost napake zaradi asinhronega clocka
        run=TBR;  // tekoci cas (timer B) ponovno branje
    run=(run+3)&0xfffc;  // tekoci cas (timer B) , kvantiziran na 1ms
    TimTime[t_appID]=tim+run;   // tabela casov sprozitve
    // dodajanje v tabelo zaporedja
    if (IorderZ==0)   // dodajanje na konec tabele zaporedja
    {
        i=IorderK-1;  // konec v tabeli zaporedja
        while ((tim<(TimTime[TimOrder[i]]-run)) && (i>=0))
        {
            TimOrder[i+1]=TimOrder[i];
            i--;
        }
        i++;
        IorderK++;  // konec v tabeli zaporedja
    }
    else   // dodajanje na zacetek tabele zaporedja
    {
        i=IorderZ;  // zacetek v tabeli zaporedja
        while ((tim>=(TimTime[TimOrder[i]]-run)) && (i<IorderK))
        {
            TimOrder[i-1]=TimOrder[i];
            i++;
        }
        i--;
        IorderZ--;  // zacetek v tabeli zaporedja
    }
    TimOrder[i]=t_appID;
    if (i==IorderZ) // sprememba na zacetku tabele
    {
        TBCCTL0 &= ~CCIFG;       // brise IRQ status
        tim = TimTime[TimOrder[i]];
        TBCCR0 = tim;
        
        //    if (tim==TAR)  // tekoci cas (timer B)
        //      TACCR0 = tim+1;   // prozenje v primeru preskoka timerja
    }
    
    TBCCTL0 |= CCIE;        // TRCCR0 interrupt enabled
}

void timer_wait_off(unsigned int t_appID)
{
    unsigned int Tmask;
    int i;
    
    t_appID--;   // 1..16 -> 0..15
    if (t_appID >= MaxTim)
        return; //  neveljaven index
    Tmask=BitMaskW[t_appID];
    if ((TimEnFlg & Tmask)==0)   // ne obstaja
        return;
    //  TACCTL0 &= ~CCIE;       // TRCCR0 interrupt disabled
    TimEnFlg &= ~Tmask;
    // izlocanje iz tabele zaporedja
    i=IorderZ;  // zacetek v tabeli zaporedja
    while ((TimOrder[i]!=t_appID) && (i<IorderK))
        i++;
    TBCCTL0 &= ~CCIE;       // TRCCR0 interrupt disabled
    if (i<IorderK)
    {
        while (i<IorderK)
        {
            TimOrder[i]=TimOrder[i+1];
            i++;
        }
        IorderK--;
    }
    TBCCTL0 |= CCIE;        // TRCCR0 interrupt enabled
}



// ---------------------------------------------------------------------------
// Timer B0 interrupt service routine
#pragma vector=TIMERB0_VECTOR
__interrupt void Timer_B0(void)
{
	unsigned int Tmask, tim, t_appID;
	
	//_cpu_on();
	
	TrigTim = 0;
	
	do {
		t_appID=TimOrder[IorderZ];  // zacetek v tabeli zaporedja
		Tmask=BitMaskW[t_appID];
		if (TimEnFlg & Tmask)   // obstaja
		{
			TrigTim |= Tmask;   // ob poteku sztevca se postavi pripadajoci bit, aplikacije brisejo pripadajoce bite
			//    apps_call(_int_to_flag(T_index));  // "helper.h" vsebuje _int_to_flag  // "scheduler.h" vsebuje apps_call
			TimEnFlg &= ~Tmask;
			
			apps_call(Tmask);  // "helper.h" vsebuje _int_to_flag  // "scheduler.h" vsebuje apps_call
		}
		if (IorderZ<IorderK)
		{
			IorderZ++;
			if (IorderZ<IorderK)
				tim = TimTime[TimOrder[IorderZ]];
		}
	} while ((tim==TBCCR0) && (IorderZ<IorderK));   // brez moznosti povecanja TBR preko kvantizacije med obdelavo
	//  } while (((tim==TBCCR0)||(tim==TBR)) && (IorderZ<IorderK)); // TBR se med obdelavo lahko poveca za 1
	if (TimEnFlg)   // obstaja naslednji
	{
		TBCCR0 = tim;
	}
	else
		TBCCTL0 &= ~CCIE;       // TRCCR0 interrupt disabled
	
	
	//  if (TrigTim)
	//    __bic_SR_register_on_exit(LPM3_bits);        // Exit LPM3
    __low_power_mode_off_on_exit();
	
	//_cpu_off_irq();
	
}


// ---------------------------------------------------------------------------
// rutine za sekundne intervale
// ---------------------------------------------------------------------------

void s_timer_wait(unsigned int t_appID, unsigned int sec)
{
    // t_appID index 0..15
    unsigned int Tmask;
    
    t_appID--;   // 1..16 -> 0..15
    if (t_appID >= MaxTim)
        return; //  neveljaven index
    if (sec==0)
        return;   // nedovoljena vrednost
    Tmask=BitMaskW[t_appID];
    if (STimEnFlg & Tmask)   // ze obstaja
        s_timer_wait_off(t_appID+1);  // prekinitev obstojecega
    STimEnFlg^=Tmask;
    _DINT();  // prepove prekinitve (da se SecStev ne spremeni)
    STimTime[t_appID]=SecStev+sec;   // tabela casov sprozitve
    _EINT();  // dovoli prekinitve
}

void s_timer_wait_off(unsigned int t_appID)
{
    unsigned int Tmask;
    
    t_appID--;   // 1..16 -> 0..15
    if (t_appID >= MaxTim)
        return; //  neveljaven index
    Tmask=BitMaskW[t_appID];
    //  if ((STimEnFlg & Tmask)==0)   // ne obstaja
    //    return;
    STimEnFlg &= ~Tmask;
}

// ---------------------------------------------------------------------------
const unsigned char MesDni[12]={31,29,31,30,31,30,31,31,30,31,30,31};
// sekundna prekinitev ( obdelava sekundnih timerjev in RTC)
void SecIRQ(void)
{
    unsigned int i,Tmask;
    // sekundni timerji
    for (i=0;i<MaxTim;i++)
    {
        if (STimTime[i]==SecStev)
        {
            Tmask=BitMaskW[i];
            if (STimEnFlg & Tmask)   // obstaja
            {
                STrigTim |= Tmask;  // ob poteku sztevca se postavi pripadajoci bit, aplikacije brisejo pripadajoce bite
                //    apps_call(_int_to_flag(T_index));  // "helper.h" vsebuje _int_to_flag  // "scheduler.h" vsebuje apps_call
                STimEnFlg &= ~Tmask;
                apps_call(Tmask);  // "helper.h" vsebuje _int_to_flag  // "scheduler.h" vsebuje apps_call
                
            }
        }
    }
    // RTC
    RTCsec++;    // sekunde, minute, ure, dnevi, mesci, leta
#if FastMin
    if (RTCsec>=FastMin)
#else
        if (RTCsec>=60)
#endif
        {
            RTCsec=0;
            TrigMin=TimEnMin;   // vsako minuto se vpise TimEnMin, aplikacije brisejo pripadajoce bite
            apps_call(TimEnMin);   // zagon aplikacij ( v1.5 +)
            RTCabsMin++; // število minut od nedelje, 31.12.2000
            RTCmin++;   // minute
            
            //       pulse[0].RAWACC_FREEZE = pulse[0].RAWACC;
            //       pulse[1].RAWACC_FREEZE = pulse[1].RAWACC;
            //       pulse[2].RAWACC_FREEZE = pulse[2].RAWACC;
            //       pulse[3].RAWACC_FREEZE = pulse[3].RAWACC;
            
            if (RTCmin>=60)
            {
                RTCmin=0;
                RTChour++;   // ure
                if (RTCdvt==0)   // preverjanje za avtomatski preklop poletni/zimski cas
                    if (RTCctr&STauto) 	// bit-0: dovoljena avtomatska sprememba
                        if (RTCday>=25)    // zadnja nedelja v mescu
                        {
                            if (RTCmonth==3)   // marec
                            {
                                if (RTChour==2)  // preklop na poteni cas
                                {
                                    /* Alarm Old */
                                    //_alarm_clock_old();
                                    
                                    RTChour++;
                                    RTCctr |=STbit;  // status za poletni cas
                                    
                                    /* added */
                                    RTCabsMin+= 60;
                                    
                                    /* LOGGER FIX */
                                    RTCctr |= TimeCh;   // status za spemenjeno uro
                                    
                                    /* Alarm New */
                                    //_alarm_clock_new();
                                }
                            }
                            else
                                if (RTCmonth==10)   // oktober
                                {
                                    if (RTCctr & STbit)  // status za poletni cas
                                        if (RTChour==3)    // preklop na zimski cas
                                        {
                                            /* Alarm Old */
                                            //_alarm_clock_old();
                                            
                                            RTChour--;
                                            RTCctr &=~STbit;  // status za zimski cas
                                            
                                            /* added */
                                            RTCabsMin-= 60;
                                            
                                            /* LOGGER FIX */
                                            RTCctr |= TimeCh;   // status za spemenjeno uro
                                            
                                            /* Alarm New */
                                            //_alarm_clock_new();
                                        }
                                }
                        }
                if (RTChour>=24)
                {
                    RTChour=0;
                    RTCday++;  // dnevi
                    RTCdvt++;  // dan v tednu 0..6
                    if (RTCdvt>=7)
                        RTCdvt=0;
                    RTCabsDay++; // stevilo dni od nedelje, 31.12.2000
                    i=MesDni[RTCmonth-1];
                    if (RTCmonth==2)
                        if (RTCyear&3)
                            i--;   // ni prestopno leto
                    if (RTCday>i)
                    {
                        RTCday=1;
                        RTCmonth++;   // mesci
                        if (RTCmonth>=13)
                        {
                            RTCmonth=1;
                            RTCyear++;   // leta
                            if (RTCyear>=100)
                                RTCyear=0;
                        }
                    }
                }
            }
        }
}
// ---------------------------------------------------------------------------

// Timer_B7 Interrupt Vector (TAIV) handler
#pragma vector=TIMERB1_VECTOR
__interrupt void Timer_B1(void)
{
    
    //_cpu_on();
    
    switch( TBIV )
    {
    case  2:
        
        break;                          // CCR1 not used
    case  4:
        
        break;                          // CCR2
        
    case  6:
        break;                          // CCR3
    case  8:
       
        break;                          // CCR4
        
        // *** TACCR5 ***
    case  10:                                 // TimerB CCR5 is used for Power Manager
        
        break;
        
        // *** CCR6 ***
    case  12:
         _EINT();  // dovoli ostale prekinitve
        TBCCR6 += 32768U/8;   // 1s interval
        TrigSec=TimEnSec;    // vsako sekundo se vpise TimEnSec, aplikacije brisejo pripadajoce bite
        apps_call(TimEnSec);   // zagon aplikacij ( v1.5 +)
        SecStev++;   // stevec sekund
        SecIRQ();    // obdelava sekundnih timerjev in RTC
        
        __bic_SR_register_on_exit(LPM3_bits);        // Exit LPM3
        break;
        // ***
    case 14:                   // overflow not used
        break;
    }
    
    //_cpu_off_irq();
    
}

// ---------------------------------------------------------------------------
// *** rutine za RTC ***
// ---------------------------------------------------------------------------
const unsigned int zd[12]={0,31,59,90,120,151,181,212,243,273,304,334};
// izracun stevila dni od nedelje, 31.12.2000
/*
int AbsDan(unsigned int dan, unsigned int mes, unsigned int let)
{
int d;

mes--;
if (mes>=12)
mes=0;
d=(let-1)*365+zd[mes]+dan;
if (mes<2)
let--;
d+=(let/4);
return d;
} */
int AbsDanC(unsigned char *RTCmb)
{
    int d;
    unsigned int dan, mes, let;
    
    dan=RTCmb[0];
    let=RTCmb[2];
    mes=RTCmb[1]-1;
    if (mes>=12)
        mes=0;
    d=(let-1)*365+zd[mes]+dan;
    if (mes<2)
        let--;
    d+=(let/4);
    return d;
}

// izracun datuma iz stevila dni od nedelje, 31.12.2000
unsigned int Abs2binDate(int d)
{
    unsigned int let,mes;
    
    if (d<1)
        return 0;
    let=d/366;
    d-=(let*365)+(let/4); // od zacetka leta
    let++; 	// od 2000
    mes=11;
    while (d<=zd[mes])
        mes--;
    d-=zd[mes];
    if (mes>1)
        if ((let&3)==0)
        {
            d--; 	// prestopno leto
            if (d==0)
            {
                mes--;
                if (mes>1)
                    d=zd[mes+1]-zd[mes];
                else
                    d=29;
            }
        }
    if (d>31)
    {
        d-=31;
        let++;
        mes=0;
    }
    mes++; 	// 1..12
    return (unsigned long)(d|(mes<<5)|(let<<9))<<1;
}

void CopyLong3Char(unsigned long t, unsigned char *RTCmb)
{
    RTCmb[0]=t;
    RTCmb[1]=t>>8;
    RTCmb[2]=t>>16;
}

unsigned int PackDate(unsigned char *RTCmb) 	// pretvorba datuma v binarni format
{
    unsigned int u,t;
    u=RTCmb[0]; 	// dan
    if (u>31)
        u=0;
    t=u<<1;
    u=RTCmb[1]; 	// mesec
    if (u>12)
        u=0;
    t|=u<<6;
    //	u=RTCmb[2]&15; 		// leto
    u=RTCmb[2]; 		// leto
    t|=u<<10;
    return t;
}

unsigned long UnpackDate(unsigned int t) 	// pretvorba datuma iz binarnega formata
{
    unsigned long b;
    unsigned int i;
    
    t=t>>1;
    //	b=(t&0x1f)<<8;	// dan  * v1.5- *
    b=(t&0x1f);	// dan
    //	b|=((t>>5)&0x0f);	// mesec * v1.5- *
    b|=(((t>>5)&0x0f))<<8;	// mesec
    //	i=(t>>9)&0x0f; 		// leto % 16;
    //	i+=RTCyear&0xf0;
    //	if (i>RTCyear)
    //		i-=16;
    i=(t>>9)&0x3f; 		// leto % 64;
    b|=(unsigned long)i<<16;
    return b;
}

unsigned long PackTime(unsigned char *RTCmb) 	// pretvorba casa v binarni format v sekundah
{
    unsigned long t;
    //	unsigned int u;
    t=RTCmb[2]*60; 	// ure
    t+=RTCmb[1]; // minute
    t*=60;
    t+=RTCmb[0]; 		// sekunde
    return t|((unsigned long)PackDate(&RTCmb[3])<<16);
}

unsigned long UnpackTime(unsigned long t) 	// pretvorba casa iz binarnega formata
{
    unsigned long b;
    unsigned int i;
    
    t&=0x1ffff; 	// sekunde brez datuma
    i=t/3600;
    t-=i*3600L;
    b=i;	// ure
    b<<=16;
    i=t/60;
    t-=i*60;
    b|=i<<8;	//minute
    b|=t;	// sekunde
    return b;
}


void clock_offset(signed int sec_off)   // premik èasa v sekundah
{
    long t;
    
    //	BeriRTC();
    t=PackTime((unsigned char *)&RTCsec)&0x1ffff;	// brez datuma
    if ( sec_off > 0)	// premik ure naprej
    {
        t+= sec_off;
        if (t>86400)  	// premik dneva
        {
            t-=86400;
            //						LcopyI(Bin2bcdDate(Abs2binDate(RTCdan+1)),&RTCmb[2]);
            CopyLong3Char(UnpackDate(Abs2binDate(AbsDanC((unsigned char *)&RTCday)+1)),(unsigned char *)&RTCday);	// spremenjena datuma
        }
    }
    else
    {
        t+= sec_off;
        if (t<0)  	// premik dneva
        {
            t+=86400;
            //						LcopyI(Bin2bcdDate(Abs2binDate(RTCdan-1)),&RTCmb[2]);
            CopyLong3Char(UnpackDate(Abs2binDate(AbsDanC((unsigned char *)&RTCday)-1)),(unsigned char *)&RTCday);	// spremenjena datuma
        }
    }
    CopyLong3Char(UnpackTime(t),(unsigned char *)&RTCsec);	// spremenjena ura
    
}




// ---------------------------------------------------------------------------
int TestST(int set)   // test za poletni cas
{
    unsigned int i,dan, mes, dvt, ur;
    int d;
    long t;
    
    i=RTCctr&STbit; 	// trenutno stanje ST
    mes=RTCmonth; 	// RTC.mesec;
    if ((mes<3)||(mes>10))
        i=0; 	// zimski cas
    else
    {
        if ((mes>3)&&(mes<10))
            i=STbit; 	// poletni cas
        else
        {
            dan=RTCday; //BCD2BIN((RTCmb[2]>>8)&0x3f);  //dan=BCD2BIN(RTC.dan);
            //			dvt=RTC.dvt;	// 0=nedelja
            //	dvt=(AbsDan(dan, mes, RTCyear))%7;
            dvt=(AbsDanC((unsigned char *)&RTCday))%7;
            RTCdvt=dvt;   // dodatna ponastavitev dneva v tednu
            d=dan-dvt;
            while (d<=31-7)
                d+=7; 	// zadnja nedelja v mescu
            ur=RTChour;			
            if (mes==3)
            {
                if (dan<d)
                    i=0; 	// zimski
                else
                {
                    if (dan>d)
                        i=STbit; 	// poletni
                    else
                    {
                        if (ur<2)
                            i=0;
                        else
                            i=STbit;
                    }
                }
            }
            else
            {
                if (dan<d)
                    i=STbit; 	// poletni
                else
                {
                    if (dan>d)
                        i=0; 	// zimski
                    else
                    {
                        if (ur<2)
                            i=STbit; 	//poletni
                        else
                        {
                            if (ur>=3)
                                i=0;	// zimski
                        }
                    }
                }
            }
        }
    }
    if (set)	// izvrsitev spremembe
        //		if (i!=RTC.ST) 		 	// sprememba
        if (i!=(RTCctr&STbit))          // bit-1: stanje za poletni cas
            if (RTCctr&STauto) 	// bit-2: dovoljena avtomatska sprememba
            {
                /* Alarm Old */
                //_alarm_clock_old();
                
                //	BeriRTC();
                t=PackTime((unsigned char *)&RTCsec)&0x1ffff;	// brez datuma
                if (i)	// premik ure naprej
                {
                    t+=3600;
                    if (t>86400)  	// premik dneva
                    {
                        t-=86400;
                        //						LcopyI(Bin2bcdDate(Abs2binDate(RTCdan+1)),&RTCmb[2]);
                        CopyLong3Char(UnpackDate(Abs2binDate(AbsDanC((unsigned char *)&RTCday)+1)),(unsigned char *)&RTCday);	// spremenjena datuma
                    }
                }
                else
                {
                    t-=3600;
                    if (t<0)  	// premik dneva
                    {
                        t+=86400;
                        //						LcopyI(Bin2bcdDate(Abs2binDate(RTCdan-1)),&RTCmb[2]);
                        CopyLong3Char(UnpackDate(Abs2binDate(AbsDanC((unsigned char *)&RTCday)-1)),(unsigned char *)&RTCday);	// spremenjena datuma
                    }
                }
                CopyLong3Char(UnpackTime(t),(unsigned char *)&RTCsec);	// spremenjena ura
                if (i)
                    RTCctr |=STbit;
                else
                    RTCctr &=~STbit;
                
                /* Alarm New */
                //_alarm_clock_new();
            }
    return i;
}
// ---------------------------------------------------------------------------
// osvezi STbit glede na trenutni cas
void UpdateST(void)
{
    if (TestST(0))   // test za poletni cas
        RTCctr |=STbit; 	// trenutno stanje ST poletni cas
    else
        RTCctr &=~STbit; 	// trenutno stanje ST zimsni cas
}

// nastavi avtomatski prekolp poletni/zimski cas (STauto)
void UpdateDST(char d)
{
    if (d)
        RTCctr |=STauto;
    else
        RTCctr &=~STauto;
    UpdateST();     // osvezi STbit glede na trenutni cas
}
// ---------------------------------------------------------------------------
// nastavitev datuma dan (0:7), mesec (8:15) in leta ter izracun dbeva v tednu in absolutni dan
void RTC_date_set(unsigned int date, unsigned char year)
{
    RTCdate=date;
    RTCyear=year;
    RTCabsDay=AbsDanC((unsigned char *)&RTCday); // izracun stevila dni od nedelje, 31.12.2000
    RTCdvt=RTCabsDay%7;         // izracun dneva v tednu
    UpdateST();     // osvezi STbit glede na trenutni cas
    RTCctr |= DateCh;   // status za spemenjen datum
    RTCabsMin = AbsMinC((unsigned char *)&RTCmin); // število minut od nedelje, 31.12.2000
}

// nastavitev casa minute (0:7), ure (8:15) in sekund ter sinhroniczacija sekundnih IRQ
void RTC_time_set(unsigned int time, unsigned char sec)
{
    TBCCR6 = TBR+(32768U/8);   // 1s interval
    RTCsec=sec;
    RTCtime=time;
    UpdateST();     // osvezi STbit glede na trenutni cas
    RTCctr |= TimeCh;   // status za spemenjeno uro
    RTCabsMin = AbsMinC((unsigned char *)&RTCmin); // število minut od nedelje, 31.12.2000
}


// nastavitev ure in datuma
void RTC_clock_set(unsigned char *RTCmb)
{
    TBCCR6 = TBR+(32768U/8);   // 1s interval
    
    /* Alarm Old */
    //_alarm_clock_old();
    
    RTCsec = RTCmb[0];
    RTCmin = RTCmb[1];
    RTChour = RTCmb[2];
    RTCctr |= TimeCh;   // status za spemenjeno uro
    RTC_date_set(((unsigned int)RTCmb[4]<<8)|RTCmb[3], RTCmb[5]);
    RTCabsMin = AbsMinC((unsigned char *)&RTCmin); // število minut od nedelje, 31.12.2000
    
    /* Alarm New */
    //_alarm_clock_new();
}

// nastavitev ure in datuma z opcijskim premikom na zimski cas
void GSM_clock_sync(unsigned char *RTCmb)
{
    long t;
    
    RTC_clock_set(RTCmb);
    if ((RTCctr & STauto) == 0)   // vedno zimski cas
        if (RTCctr & STbit) 	// trenutno stanje ST poletni cas
        { // premik ure nazaj
            t=PackTime((unsigned char *)&RTCsec)&0x1ffff;	// brez datuma
            t-=3600;
            if (t<0)  	// premik dneva
            {
		t+=86400;
                CopyLong3Char(UnpackDate(Abs2binDate(AbsDanC((unsigned char *)&RTCday)-1)),(unsigned char *)&RTCday);	// spremenjena datuma
            }
            CopyLong3Char(UnpackTime(t),(unsigned char *)&RTCsec);	// spremenjena ura
            //      RTCabsMin = AbsMinC((unsigned char *)&RTCmin); // število minut od nedelje, 31.12.2000
            RTCabsMin -= 60;  // samo sprememba za eno uro nazaj
            
            /* Alarm New */
            //_alarm_clock_new();
        }
}

// ---------------------------------------------------------------------------
long AbsMinC(unsigned char *RTCmb)
{
    long m;
    unsigned int min,ur,dan, mes, let;
    min=RTCmb[0];
    ur=RTCmb[1];
    dan=RTCmb[2];
    mes=RTCmb[3]-1;
    let=RTCmb[4];
    
    
    if (mes>=12)
        mes=0;
    m=(let-1)*365+zd[mes]+dan;
    if (mes<2)
        let--;
    m+=(let/4);
    m = (m*60*24) + (ur*60) + min;
    return m;
}
// ---------------------------------------------------------------------------

// testira èe je èas znotraj

unsigned char RTC_ckeck(void){
    
    if (RTCmin < 60){
        if(RTChour < 24){
            if(RTCday < 32){
                if(RTCmonth < 13){
                    if((RTCyear < 99) && (RTCyear > 8)){
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
    
    
}
