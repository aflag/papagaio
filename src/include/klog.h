#ifndef _KLOG__H
#define _KLOG__H

int klog(int metodo, const char *s, ...)
	__attribute__ ((format (printf, 2, 3)));

enum metodos {
	CONSOLE
};

#endif
