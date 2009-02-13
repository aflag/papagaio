#ifndef __MALLOC_H
#define __MALLOC_H
/* public domain */

#include <tipos.h>

void inicializa_malloc(void);
void free(void *ptr);
void* malloc(u32 tamanho);

#endif
