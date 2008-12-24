#include <console.h>
#include <klog.h>

void kmain(void)
{
	int i=0;

	inicia_console();
	for (i = 0; i < 10; ++i)
		klog(CONSOLE, "curupacu\n");
	for (i = 0; i < 10; ++i)
		klog(CONSOLE, "paracatu\n");
	for (i = 0; i < 10; ++i)
		klog(CONSOLE, "blurp\n");
}
