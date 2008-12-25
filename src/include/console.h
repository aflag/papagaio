#ifndef _CONSOLE__H
#define _CONSOLE__H

#include <stdarg.h>

void vimprime(const char *s, va_list ap);

void imprime(const char *s, ...) __attribute__ ((format (printf, 1, 2)));

#endif
