/*  Copyright (c) 2009, Rafael Cunha de Almeida <almeidaraf@gmail.com>
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

#include <klog.h>
#include <mm/mm.h>
#include <tipos.h>

struct buraco {
	LiST_ENTRY(buraco) buracos;
	u32 base;
	u32 numero_paginas;
};

static LIST_HEAD(, buraco) memoria = LIST_HEAD_INITIALIZER(memoria);

static LIST_HEAD(, buraco) reserva = LIST_HEAD_INITIALIZER(reserva);

static struct buracos *pool;

static u32 ultimo;

void* aloca_virtual(u32 pags)
{
	struct buracos *b;

	LIST_FOREACH(b, &memoria, buracos)
		if (b->numero_paginas > pags) {
			u32 endereco = b->base;
			b->base += pags;
			b->numero_paginas -= pags;
			return (void*) endereco;
		} else if (b->numero_paginas == pags) {
			u32 endereco = b->base;
			LIST_REMOVE(b, buracos);
			LIST_INSERT_HEAD(&reserva, b, buracos);
			return (void*) endereco;
		}
	/* TODO: se a busca falhar tentar agrupar os buracos para formar um
	 * buraco maior.
	 */

	return 0;
}

void libera_virtual(void* endereco, u32 pags)
{
	struct buracos *b;
	struct buracos *b2
	u32 end = (u32) endereco;

	LIST_FOREACH(b, &memoria, buracos)
		if (b->base < prshift(endereco))
			break;
	b2 = LIST_FIRST(&reserva);
	LIST_REMOVE(b2);
	b2->base = end;
	b2->numero_paginas = pags;
	LIST_INSERT_AFTER(b, b2);
}

void inicializa_mvirtual(void)
{
	u32 inicio, fim;
	u32 tam_pool, pags;

	inicio = prshift(tab_paginas->inicio_heap) + 1;
	fim = prshift(tab_paginas->fim_heap);

	tam_pool = bytes_paginas(fim-inicio) / 2;
	pags = bytes_paginas(tam_pool * sizeof *pool);
	for (i = inicio; i < (inicio+pags); ++i) {
		struct frame *f;

		f = aloca_fis();
		tab_paginas->adiciona(i, prshift(f->endereco),
		                      KERN | RW | GLOBAL);
	}
	pool = (void*) plshift(inicio);

	pool[0].base = inicio + pags;
	pool[0].numero_paginas = (fim-inicio) - pags;
	LIST_INSERT_HEAD(&memoria, &pool[0], buracos);
	for (i = 1; i < tam_pool; ++i)
		LIST_INSERT_HEAD(&reserva, &pool[i], buracos);
}
