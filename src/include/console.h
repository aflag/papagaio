#ifndef __CONSOLE_H
#define __CONSOLE_H
/* public domain */

#include <stdarg.h>

void vimprime(const char *s, va_list ap);

void imprime(const char *s, ...) __attribute__ ((format (printf, 1, 2)));

#endif
