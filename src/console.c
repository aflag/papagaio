/*  Copyright (c) 2008, 2009, Rafael Cunha de Almeida <almeidaraf@gmail.com>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
 
#include <console.h>

#define NUM_COLUNAS 80
#define NUM_LINHAS 25

/* O tamanho do buffer de video é 80 colunas por 25 linhas. Cada caractere pode
 * ter uma cor diferente e, por isso, existem 2 bytes associados a cada caracter
 * na tela.
 */
#define TAMANHO (NUM_LINHAS*NUM_COLUNAS*2)

static unsigned int proxima_linha = 0;
static unsigned int proxima_coluna = 0;

/* A memória 0xb8000 está mapeada na memória de video colorido. Todos PCs que eu
 * tenho acesso funcionam dessa maneira.
 */
static char *memoria_vid = (char*)0xb8000;

#define COR 0x02

static void escreve_pos(int linha, int coluna, char c)
{
	int pos = 2*linha*NUM_COLUNAS + 2*coluna;

	memoria_vid[pos] = c;
	++pos;
	memoria_vid[pos] = COR;
}

static char le_pos(int linha, int coluna)
{
	return memoria_vid[2*linha*NUM_COLUNAS + 2*coluna];
}

static void sobe_linha()
{
	unsigned int i, j;
	char c;

	for (i = 1; i < NUM_LINHAS; ++i)
		for (j = 0; j < NUM_COLUNAS; ++j) {
			c = le_pos(i, j);
			escreve_pos(i-1, j, c);
		}

	proxima_coluna = 0;
	--proxima_linha;
}

void inicia_console(void)
{
	int i;

	for (i = 0; i < TAMANHO; ++i)
		if (!(i%2))
			memoria_vid[i] = 0x0;
		else
			memoria_vid[i] = COR;
}

int imprime(const char *s)
{
	unsigned int i;
	char c;

	while (c = *s) {
		if (proxima_coluna >= NUM_COLUNAS) {
			proxima_coluna = 0;
			++proxima_linha;
		}

		if (proxima_linha >= NUM_LINHAS)
			sobe_linha();

		if (c == '\n') {
			proxima_coluna = 0;
			++proxima_linha;
		} else {
			escreve_pos(proxima_linha, proxima_coluna, c);
			++proxima_coluna;
		}

		++s;
	}
}
