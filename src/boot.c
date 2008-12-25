#include <console.h>
#include <klog.h>

void kmain(void)
{
	int i=0;

	for (; i< 25; ++i)
		klog(CONSOLE, "cagada %x <<---\n", i);
}
