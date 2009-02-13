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

#define NUM_LISTAS 7
#define MENOR_POTENCIA 6

static LIST_HEAD(, frame) listas[NUM_LISTAS]; 

struct pagina {
	LIST_ENTRY(pagina) paginas;
	u32 endereco;
};
static LIST_HEAD(, pagina) paginas_livres =
	LIST_HEAD_INITIALIZER(paginas_livres);

static struct pagina *paginas;

static inline u32 indice(u32 necessario)
{
	u32 possivel, i;

	i = 0;
	do {
		possivel = 1 << (i+MENOR_POTENCIA);
		++i;
	} while (possivel < necessario);

	return i-1;
}

static inline u16 cheio(struct frame *f)
{
	return f->offset >= TAM_FRAME;
}

static inline u16 tamanho_bloco(u16 indice)
{
	return 1 << (indice+MENOR_POTENCIA);
}

static void* aloca_bloco(struct frame *f, u16 aloc_tam)
{
	u32 endereco;

	if (TAM_FRAME < (f->offset + aloc_tam))
		return 0;

	endereco = ((struct pagina*)f->meta_pagina)->endereco;
	endereco += f->offset;

	f->offset += aloc_tam;
	++f->contador;

	return (void*)endereco;
}

static struct pagina* aloca_pagina_virtual(void)
{
	struct pagina *p;

	if (LIST_EMPTY(&paginas_livres))
		return 0;
	
	p = LIST_FIRST(&paginas_livres);
	LIST_REMOVE(p, paginas);

	return p;
}

static void* aloca_novo_frame(u16 indice)
{
	struct frame *f;
	struct pagina *p;

	f = aloca_fis();
	if (!f)
		return 0;

	p = aloca_pagina_virtual();
	if (!p) {
		libera_fis(f);
		return 0;
	}
	f->meta_pagina = p;
	f->offset = 0;
	f->contador = 0;

	tab_paginas->adiciona(prshift(p->endereco), prshift(f->endereco),
	                      KERN | RW | GLOBAL);
	tab_paginas->flush_endereco((void*)p->endereco);

	LIST_INSERT_HEAD(&listas[indice], f, frames);

	return aloca_bloco(f, tamanho_bloco(indice));
}

void* malloc(u32 bytes)
{
	u32 i, tamanho;
	void *bloco;
	struct frame *f;

	i = indice(bytes);
	tamanho = tamanho_bloco(i);

	LIST_FOREACH(f, &listas[i], frames)
		if (!cheio(f)) {
			bloco = aloca_bloco(f, tamanho);
			goto bloco_alocado;
		}

	bloco = aloca_novo_frame(i);

bloco_alocado:
	if (0 == bloco)
		return 0;
	return bloco;
}

void free(void *ptr)
{
}

void inicializa_malloc(void)
{
	u32 i;
	u32 inicio, fim;

	for (i = 0; i < NUM_LISTAS; ++i)
		LIST_INIT(listas+i);

	inicio = prshift(tab_paginas->inicio_heap) + 1;
	fim = prshift(tab_paginas->fim_heap);

	u32 no_paginas = bytes_paginas(fim-inicio);
	u32 tam_array = bytes_paginas(no_paginas * sizeof *paginas);
	for (i = inicio; i < (inicio+tam_array); ++i) {
		struct frame *f;

		f = aloca_fis();
		tab_paginas->adiciona(i, prshift(f->endereco),
		                      KERN | RW | GLOBAL);
	}
	paginas = (void*) plshift(inicio);

	u32 p = inicio;
	for (i = 0; i < no_paginas; ++i, ++p) {
		paginas[i].endereco = plshift(p);
		if (p > tam_array)
			LIST_INSERT_HEAD(&paginas_livres, &paginas[i], paginas);
	}
}
