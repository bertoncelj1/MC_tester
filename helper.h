/* HELPER version 1.7 */

#ifndef __HELPER_H__
#define __HELPER_H__

/* create flag - convert (unsigned) integer to bit-flag */
#define _int_to_flag(_i)	(1 << (_i - 1))
#define _flag(_i)		(1 << _i)

/* helpers for local app suspend */
#define _left_to_flag(_i)	(((unsigned int) 0x8000) >> (_i - 1))
#define _uleft_to_flag(_i)	(((signed int) 0x8000) >> (_i - 1))

#define _test_flag_int(_p, _i)	(_p & _int_to_flag(_i))
#define _set_flag_int(_p, _i)	_p |= _int_to_flag(_i)
#define _clear_flag_int(_p, _i)	_p &= ~_int_to_flag(_i)


/* type ulong increment/decrement/zero */
#define _32bit_inc(_p)		(*(unsigned long *)&_p)++
#define _32bit_dec(_p)		(*(unsigned long *)&_p)--
#define _32bit_zero(_p)		(*(unsigned long *)&_p) = 0

inline void _mult_10(unsigned long *p)
{
  unsigned long t;

  t = *p;
  *p = *p << 2;	//x4
  *p += t;	//+ p
  *p = *p << 1;	//x2
}

int fromHexToByte(unsigned char* cs, unsigned char* b);
void fromByteToHex(unsigned char* b, unsigned char* cs);

void clear_string(char *pchar, int len);
void clear_string_ascii(char *pchar, int len);

void delay(unsigned int msec);

#endif /*__HELPER_H__*/
