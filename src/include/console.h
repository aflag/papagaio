#ifndef _CONSOLE__H
#define _CONSOLE__H

#include <stdarg.h>

void vimprime(const char *s, va_list ap);

void imprime(const char *s, ...);

void inicia_console(void);

enum erros {
	E_TAMANHO = -20
};

#endif
