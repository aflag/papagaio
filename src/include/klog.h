#ifndef __KLOG_H
#define __KLOG_H
/* public domain */

int klog(int prioridade, const char *s, ...)
	__attribute__ ((format (printf, 2, 3)));

#define debug(fmt, args...) klog(DEBUG, fmt, ## args)

enum prioridades {
	ERRO,
	ALERTA,
	NOTA,
	DEBUG
};

#endif
