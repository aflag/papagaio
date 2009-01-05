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

#include <bit.h>
#include <tipos.h>
#include <multiboot.h>
#include <klog.h>
#include <vm.h>

/* Final do kernel contando apenas o código e conteúdo estático (colocado na
 * tabela de simbolos pelo linker).
 */
extern int final_estatico;
/* Final do kernel com o código, conteúdo estático e mais a tabela de frames
 * livres.
 */
static u32 fim_kernel;

static u32 *frames_livres;
static u32 proximo_frame;

/* diretorio para as tabelas de pagina */
static struct item_diretorio diretorio[N_ENTRADAS_DIRETORIO];

static __inline__ int existe_frame(void)
{
	return proximo_frame != 0;
}

static __inline__ int pop_frame(void)
{
	if (!existe_frame())
		return -1;
	else
		return frames_livres[--proximo_frame];
}

static __inline__ void adiciona_frame(u32 frame)
{
	frames_livres[proximo_frame++] = frame;
}

static __inline__ u32 __transf(void *ptr)
{
	return ((u32)ptr) & 0x003fffff;
}
static __inline__ void* __aloc(u32 bytes)
{
	u32 tmp = fim_kernel;

	fim_kernel += bytes;
	fim_kernel = (fim_kernel&0xfffff000) + 0x1000;

	return (void*) (tmp + 0xc0000000);
}

static __inline__ u32 conta_frames(struct multiboot_info *mbi)
{
	struct mmap_record *minfo = mbi->mmap;
	u32 i; 
	u32 total = 0;
	u32 tam = mbi->mmap_length / (sizeof (struct mmap_record));

	for (i = 0; i < tam; ++i)
		/* tipo 1 = memoria utilizavel */
		if (minfo[i].type == 1)
			total += minfo[i].length_low >> 12;

	return total;
}

static __inline__ u32 fim_lowmem(struct multiboot_info *mbi)
{
	struct mmap_record *minfo = mbi->mmap;
	u32 i; 
	u32 tam = mbi->mmap_length / (sizeof (struct mmap_record));

	for (i = 0; i < tam; ++i)
		if (minfo[i].type!=1)
			return minfo[i].base_low & 0xfffff000;
	return 0;
}

static void* inicializa_tabelas(u32 inicio, u32 *total_tabelas)
{
	u32 n_bytes = fim_kernel - inicio;
	u32 n_paginas = n_bytes / TAM_PAGINA;
	u32 n_tabelas = n_paginas/N_ENTRADAS_TABELA + 1;
	u32 i, tam_intervalo;
	struct item_tabela *tabelas;

	if (n_tabelas > (N_ENTRADAS_TABELA*n_tabelas - n_paginas))
		++n_tabelas;

	tabelas = __aloc((n_tabelas*N_ENTRADAS_TABELA)
	                 * sizeof (struct item_tabela));

	/* zera tabelas */
	for (i = 0; i < n_tabelas*N_ENTRADAS_TABELA; ++i) {
		tabelas[i].presente = 0;
		tabelas[i].rw = 0;
		tabelas[i].us = 0;
		tabelas[i].pwt = 0;
		tabelas[i].pcd = 0;
		tabelas[i].acessado = 0;
		tabelas[i].sujo = 0;
		tabelas[i].pat = 0;
		tabelas[i].global = 0;
		tabelas[i].avail = 0;
		tabelas[i].base = 0;
	}

	/* Paginas referentes ao buraco no meio da memória */
	tam_intervalo = (INICIO_HIGHMEM-inicio) >> 12;
	for (i = 0; i < tam_intervalo; ++i) {
		tabelas[i].rw = 1;
		tabelas[i].pcd = 1;
		tabelas[i].pwt = 1;
		tabelas[i].presente = 1;
		tabelas[i].global = 1;
		tabelas[i].base = i + (inicio>>12);
	}

	/* Paginas referentes ao kernel */
	tam_intervalo = i + ((fim_kernel-INICIO_HIGHMEM) >> 12);
	for (; i < tam_intervalo; ++i) {
		tabelas[i].rw = 1;
		tabelas[i].presente = 1;
		tabelas[i].global = 1;
		tabelas[i].base = i + (INICIO_HIGHMEM>>12);
	}

	*total_tabelas = n_tabelas;
	return tabelas;
}

static void inicializa_diretorio(struct multiboot_info *mbi)
{
	struct item_tabela *tabelas;
	u32 i, entradas;

	/* Como o kernel está localizado a partir do endereço 0xc0000000, então
	 * as 768 primeiras entradas do diretorio são relativas às páginas de
	 * usuário. As restantes são todas reservadas para o kernel.
	 */

	/* zera diretorio */
	for (i = 0; i < N_ENTRADAS_DIRETORIO; ++i) {
		diretorio[i].presente = 0;
		diretorio[i].rw = 0;
		diretorio[i].us = 0;
		diretorio[i].pwt = 0;
		diretorio[i].pcd = 0;
		diretorio[i].acessado = 0;
		diretorio[i].avl = 0;
		diretorio[i].ps = 0;
		diretorio[i].global = 0;
		diretorio[i].avail = 0;
		diretorio[i].base = 0;
	}

	/* Tabelas de usuário */
	for (i = 0; i < (HIGH_HALF>>22); ++i)
		diretorio[i].us = 1;

	/* Tabelas do kernel */
	tabelas = inicializa_tabelas(fim_lowmem(mbi), &entradas);
	for (i = (HIGH_HALF>>22); i < (HIGH_HALF>>22) + entradas; ++i) {
		diretorio[i].rw = 1;
		diretorio[i].base = __transf(tabelas) >> 12;
		tabelas += N_ENTRADAS_TABELA * sizeof (struct item_tabela);
	}
}

static int cria_tabela_livres(struct multiboot_info *mbi)
{
	struct mmap_record *minfo = mbi->mmap;
	u32 tam_mmap = mbi->mmap_length / (sizeof (struct mmap_record));
	u32 i, total_frames;

	total_frames = conta_frames(mbi);
	if (!total_frames)
		return -1;
	
	frames_livres = __aloc(total_frames * sizeof (u32));

	proximo_frame = 0;

	for (i = 0; i < tam_mmap; ++i)
		if (minfo[i].type == 1) {
			u32 base = minfo[i].base_low, tam = minfo[i].length_low;
			u32 frame = base >> 12;
			u32 ultimo = (base + tam) >> 12;

			if (base & 0x00000fff) {
				klog(ALERTA, "Memoria nao alinhada:\n");
				klog(ALERTA, "  %x\n", base);
			}

			while (frame < ultimo) {
				/* Reserva a parte já alocada pelo kernel */
				if ((0x100000>>12) <= frame
				    && frame <= (fim_kernel>>12)) {
					++frame;
					continue;
				}
				frames_livres[proximo_frame++] = frame++;
			}
		}

	return SUCESSO;
}

int inicia_vm(struct multiboot_info *mbi)
{
	if (!is_set(mbi->flags, 6))
		return -1;

	/* calcular endereço físico do final do kernel */
	fim_kernel = __transf(&final_estatico);

	inicializa_diretorio(mbi);

	/*if (SUCESSO != cria_tabela_livres(mbi))
		return -2;*/

	return 0;
}
