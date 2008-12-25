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

#include <stdarg.h>

#include <console.h>

#define NUM_COLUNAS 80
#define NUM_LINHAS 25

static unsigned int proxima_linha = 0;
static unsigned int proxima_coluna = 0;

/* A memória 0xb8000 está mapeada na memória de video colorido. Todos PCs que eu
 * tenho acesso funcionam dessa maneira.
 */
static char *memoria_vid = (char*)0xb8000;

#define COR 0x02

/* Dado uma linha e uma coluna, retorna a posição do byte caso não houvessem os
 * bytes de cor.
 */
static __inline__ int valor_absoluto(int linha, int coluna)
{
	return linha*NUM_COLUNAS + coluna;
}

/* Escreve numa posição da memória de video, sem considerar a diferença de
 * linhas e colunas. Abstrai os bytes de cor, ie. pos = 0 -> 0, pos = 1 -> 2,
 * ...
 */
static __inline__ void escreve(int pos, char c)
{
	pos *= 2;
	memoria_vid[pos++] = c;
	memoria_vid[pos] = COR;
}

/* Escreve na memória de video considerando linha e coluna. */
static __inline__ void escreve_lc(int linha, int coluna, char c)
{
	int pos = 2*valor_absoluto(linha, coluna);

	memoria_vid[pos++] = c;
	memoria_vid[pos] = COR;
}

/* Lê da memória de video considerando linha e coluna. */
static __inline__ char le_lc(int linha, int coluna)
{
	return memoria_vid[2*valor_absoluto(linha, coluna)];
}

/* limpa os caracteres da linha a partir da coluna */
static __inline__ void limpa(int linha, int coluna)
{
	int i;

	for (i = coluna; i < NUM_COLUNAS; ++i)
		escreve_lc(linha, i, 0x0);
}

/* Faz a tela subir uma linha e, assim liberar uma linha de espaço */
static void sobe_linha()
{
	unsigned int i, j;
	char c;

	for (i = 1; i < NUM_LINHAS; ++i)
		for (j = 0; j < NUM_COLUNAS; ++j) {
			c = le_lc(i, j);
			escreve_lc(i-1, j, c);
		}


	limpa(NUM_LINHAS-1, 0);

	--proxima_linha;
}

/* As funcoes imprime_int, imprime_uint e imprime_xuint transformam os numeros
 * em uma sequencia de char que sao impressos na memória de video.
 */
static void imprime_int(int x)
{
	int len = 1, i, negativo = 0;
	int q;

	if (x < 0) {
		negativo = 1;
		x = -x;
		++len;
	}

	q = x;

	while (q > 9) {
		++len;
		q /= 10;
	}

	proxima_coluna += len;
	if (proxima_coluna >= NUM_COLUNAS) {
		proxima_coluna -= NUM_COLUNAS;
		++proxima_linha;
		if (proxima_linha >= NUM_LINHAS)
			sobe_linha();
	}
	i = valor_absoluto(proxima_linha, proxima_coluna) - 1;

	do {
		escreve(i--, '0' + (x % 10));
		x /= 10;
	} while (x);

	if (negativo)
		escreve(i, '-');
}

static void imprime_uint(unsigned int x)
{
	int len = 1, i;
	unsigned int q;

	q = x;

	while (q > 9) {
		++len;
		q /= 10;
	}

	proxima_coluna += len;
	if (proxima_coluna >= NUM_COLUNAS) {
		proxima_coluna -= NUM_COLUNAS;
		++proxima_linha;
		if (proxima_linha >= NUM_LINHAS)
			sobe_linha();
	}
	i = valor_absoluto(proxima_linha, proxima_coluna) - 1;

	do {
		escreve(i--, '0' + (x % 10));
		x /= 10;
	} while (x);
}

static __inline__ char tohex(unsigned int x)
{
	if (x < 10)
		return '0' + x;
	else
		return 'a' + (x-10);
}

static void imprime_xuint(unsigned int x)
{
	int len = 3, i;
	unsigned int q;

	q = x;

	while (q > 15) {
		++len;
		q /= 16;
	}

	proxima_coluna += len;
	if (proxima_coluna >= NUM_COLUNAS) {
		proxima_coluna -= NUM_COLUNAS;
		++proxima_linha;
		if (proxima_linha >= NUM_LINHAS)
			sobe_linha();
	}
	i = valor_absoluto(proxima_linha, proxima_coluna) - 1;

	do {
		escreve(i--, tohex(x % 16));
		x /= 16;
	} while (x);

	escreve(i--, 'x');
	escreve(i, '0');
}

/* Usa a função de impressão certa de acordo com a letra após a %. Essa é uma
 * maneira bem rústica de fazer um overload :-).
 */
static __inline__ void imprime_formato(char c, va_list ap)
{
	switch (c) {
		case 'd' :
			imprime_int(va_arg(ap, int));
			break;
		case 'u' :
			imprime_uint(va_arg(ap, unsigned int));
			break;
		case 'x' :
			imprime_xuint(va_arg(ap, unsigned int));
			break;
		default:
			escreve_lc(proxima_linha, proxima_coluna, c);
			++proxima_coluna;
			break;
	}
}

/* Imprime a string passada por parâmetro na memória de video. */
void vimprime(const char *fmt, va_list ap)
{
	char c;

	while ((c = *fmt)) {
		if (proxima_linha >= NUM_LINHAS) {
			proxima_coluna = 0;
			sobe_linha();
		}

		switch (c) {
			case '\n' :
				limpa(proxima_linha, proxima_coluna);
				proxima_coluna = 0;
				++proxima_linha;
				break;
			case '%' :
				if (*(fmt+1)) {
					++fmt;
					imprime_formato(*fmt, ap);
				}
				break;
			default :
				if (proxima_coluna >= NUM_COLUNAS) {
					proxima_coluna = 0;
					++proxima_linha;
				}
				escreve_lc(proxima_linha, proxima_coluna, c);
				++proxima_coluna;
				break;
		}

		++fmt;
	}
}

void imprime(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vimprime(fmt, ap);
	va_end(ap);
}
