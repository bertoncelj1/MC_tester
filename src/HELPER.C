/* HELPER version 1.7 */

#include "helper.h"

//#include  <msp430x24x.h>
#include "msp430x54x.h"
//#include "watchdog.h"

const unsigned int BitMaskW[16]={1,2,4,8,0x10,0x20,0x40,0x80,0x100,0x200,0x400,0x800,0x1000,0x2000,0x4000,0x8000};

int fromHexToByte(unsigned char* cs, unsigned char* b){
  *b = 0;

  if((cs[0] >= '0') && (cs[0] <= '9'))
  {
    *b += (cs[0] - '0') << 4;
  }
  else if((cs[0] >= 'A') && (cs[0] <= 'F'))
  {
    *b += (cs[0] - 'A' + 10) << 4;
  }
//  else if((cs[0] >= 'a') && (cs[0] <= 'f'))
//  {
//    *b += (cs[0] - 'a' + 10) << 4;
//  }
  else
    return 0;

  if((cs[1] >= '0') && (cs[1] <= '9'))
  {
    *b += (cs[1] - '0');
  }
  else if((cs[1] >= 'A') && (cs[1] <= 'F'))
  {
    *b += (cs[1] - 'A' + 10);
  }
//  else if((cs[1] >= 'a') && (cs[1] <= 'f'))
//  {
//    *b += (cs[1] - 'a' + 10);
//  }
  else
    return 0;

  return 1;

}


/** Converts the byte *b into the string cs[0] cs[1]. */
void fromByteToHex(unsigned char* b, unsigned char* cs){
   cs[0] = "0123456789ABCDEF" [ (( *b & 0xF0) >> 4) ];
   cs[1] = "0123456789ABCDEF" [ ( *b & 0x0F) ];
}

void clear_string(char *pchar, int len)
{
  for (int i = 0; i < len; i++)
    pchar[i] = 0;
}

void clear_string_ascii(char *pchar, int len)
{
  pchar[0] = 0;
  for (int i = 1; i < len; i++)
    pchar[i] = '0';
}

// Function which implements delay for max. 65ms
void delay(unsigned int msec)
{
  unsigned int tbr_delay;   // 32-bit (unsigned integer) to avoid TBR overflow
  unsigned int tbr_cap;     // 32-bit (unsigned integer) to avoid TBR overflow

  tbr_delay = msec << 2;
  tbr_cap = TBR;
  while ((TBR-tbr_cap) < tbr_delay){
    //_watchdog_reset();
  };

}


