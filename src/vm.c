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
static struct item_diretorio diretorio[N_ENTRADAS_DIRETORIO]
	__attribute__ ((aligned(4096)));

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

/* __transf é função temporária para transformar endereço virtual em real. Ela
 * deve ser utilizadas apenas nas funções de iniciação.
 */
static __inline__ u32 __transf(void *ptr)
{
	return ((u32)ptr) & 0x003fffff;
}

static __inline__ void* __aloca(u32 bytes)
{
	u32 tmp = fim_kernel;

	fim_kernel += bytes;
	fim_kernel = (fim_kernel&0xfffff000) + 0x1000;

	return (void*) (tmp + 0xc0000000);
}

static void* inicializa_tabelas(u32 inicio, u32 *total_tabelas)
{
	u32 n_bytes = fim_kernel - inicio;
	u32 n_paginas = n_bytes / TAM_PAGINA;
	u32 n_tabelas = n_paginas/N_ENTRADAS_TABELA + 1;
	u32 i;
	struct item_tabela *tabelas;

	if (n_tabelas > (N_ENTRADAS_TABELA*n_tabelas - n_paginas))
		++n_tabelas;

	tabelas = __aloca((n_tabelas*N_ENTRADAS_TABELA)
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
	for (i = inicio>>12; i < (INICIO_HIGHMEM>>12); ++i) {
		tabelas[i].rw = 1;
		tabelas[i].pcd = 1;
		tabelas[i].pwt = 1;
		tabelas[i].global = 1;
		tabelas[i].base = i;
		tabelas[i].presente = 1;
	}

	/* Paginas referentes ao kernel */
	for (i = INICIO_HIGHMEM>>12; i < (fim_kernel>>12); ++i) {
		tabelas[i].rw = 1;
		tabelas[i].global = 1;
		tabelas[i].base = i;
		tabelas[i].presente = 1;
	}

	*total_tabelas = n_tabelas;
	return tabelas;
}

/* Esta função recebe por parâmetro a estrutura do multiboot e finaliza com o
 * diretório definitivo alocado e funcionando.
 */
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
		diretorio[i].presente = 1;
	}

	/* Fazendo o seguinte eu tenho as páginas mapeadas de 0xffc00000 até
	 * 0xffffffff
	 */
	diretorio[1023].rw = 1;
	diretorio[1023].global = 1;
	diretorio[1023].base = __transf(diretorio)>>12;
	diretorio[1023].presente = 1;

	asm volatile (
		"mov %0, %%eax;"
		"mov %%eax, %%cr3;"
		:
		: "r" (__transf(diretorio))
		: "%eax"
	);
}

static __inline__ void* pega_tp(u32 entrada_diretorio)
{
	return (void*)((entrada_diretorio<<12) | 0xffc00000);
}

/* Depois que o diretorio foi alocado, podemos utilizar esta função para alocar
 * a lista de páginas livres.
 */
static __inline__ void* __aloca_lista_frames(u32 bytes)
{
	u32 i;
	void *base;
	char *p;

	p = base = __aloca(bytes);

	for (i = 0; i < (bytes/TAM_PAGINA); ++i) {
		u32 n_tp = ((u32)p) >> 22;
		u32 n_p = (((u32)p)&0x003ff000) >> 12;
		struct item_tabela *tabela;

		if (!diretorio[n_tp].presente) {
			void *tmp = __aloca(4096);
			diretorio[n_p].rw = 1;
			diretorio[n_p].base = (0x003ff000&__transf(tmp)) >> 12;
			diretorio[n_p].presente = 1;
		}

		tabela = pega_tp(n_tp);
		tabela[n_p].rw = 1;
		tabela[n_p].global = 1;
		tabela[n_p].base = (0x003ff000&__transf(p)) >> 12;
		tabela[n_p].presente = 1;

		p += TAM_PAGINA;
	}

	return base;
}

static int cria_lista_livres(struct multiboot_info *mbi)
{
	struct mmap_record *minfo = mbi->mmap;
	u32 tam_mmap = mbi->mmap_length / (sizeof (struct mmap_record));
	u32 i, total_frames;

	total_frames = conta_frames(mbi);
	if (!total_frames)
		return -1;
	
	frames_livres = __aloca_lista_frames(total_frames * sizeof (u32));

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
	if (!mbi->flags.mmap)
		return -1;

	/* calcular endereço físico do final do kernel */
	fim_kernel = __transf(&final_estatico);

	if (SUCESSO != cria_lista_livres(mbi))
		return -2;

	inicializa_diretorio(mbi);

	return 0;
}
