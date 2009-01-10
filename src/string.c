/* public domain */

unsigned int strlen(const char *s)
{
	unsigned int len = 0;

	while (s[len])
		++len;

	return len;
}

void* memset(void *s, int c, unsigned int n)
{
	char *aux = s;
	unsigned int i;

	for (i = 0; i < n; ++i)
		aux[i] = c;
	
	return s;
}
