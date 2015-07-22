#ifndef graphics_LCD2_h
#define graphics_LCD2_h

#include <msp430.h>
#include  <stdio.h>

typedef enum {OFF, GREEN, RED}e_Led_diode_state;
void led_diode(e_Led_diode_state state);

void sahovnica(void);
void sahovnica_inverzno(void);
void pas(int i);
void zgoraj_belo();
void zgoraj_crno();
void clear_2();
void BeriKey_2(void);
char LCD_read_2(int rs);
void back_light_ON(void);
void back_light_OFF(void);
int TipkaVhod_2(void);
unsigned char KGet_2(void);

void LCD_sendC_2(void);
void LCD_init_2(void);
void LE573set_2(void);
void LCD_cmd_2(unsigned char d);
void LCD_input_port_2(void);
void LCD_output_port_2(void);

void LE573hold_2(void);
void writeP1P2_out(char data);
void writeP1P2_in(char data);
char P2P1_out(void);
char P2P1_in(void);


#else
	#error graphics_LCD2_h already defined
#endif
