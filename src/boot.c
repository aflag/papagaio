#include <console.h>
#include <klog.h>

void kmain(void)
{
	int i=0;

	for (i = 0; i < 10; ++i)
		klog(CONSOLE, "curupacu\n");
	for (i = 0; i < 10; ++i)
		klog(CONSOLE, "paracatu\n");
	for (i = 0; i < 10; ++i)
		klog(CONSOLE, "batata da perna\n");
	klog(CONSOLE, "blurp\n");
	klog(CONSOLE, "batata da perna\n");
	klog(CONSOLE, "blurp\n");
	klog(CONSOLE, "blurp\n");
	klog(CONSOLE, "batata da perna\n");
	klog(CONSOLE, "batata da perna\n");
	klog(CONSOLE, "blurp\n");
}
