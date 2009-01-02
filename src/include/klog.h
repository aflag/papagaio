#ifndef __KLOG_H
#define __KLOG_H
/* public domain */

int klog(int prioridade, const char *s, ...)
	__attribute__ ((format (printf, 2, 3)));

#define debug(fmt, ...) klog(DEBUG, fmt, __VA_ARGS__)

enum prioridades {
	IMPORTANTE,
	AVISO,
	DEBUG
};

#endif
