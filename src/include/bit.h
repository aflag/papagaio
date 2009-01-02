#ifndef __BIT_H
#define __BIT_H
/* public domain */

__inline__ int is_set(u32 n, u32 bit)
{
	return n & (1<<bit);
}

#endif
