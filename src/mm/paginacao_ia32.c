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
#include <tipos.h>
#include <string.h>
#include <multiboot.h>
#include <mm/mm.h>
#include <klog.h>

struct item_diretorio {
	u32 presente:1;
	u32 rw:1;
	u32 us:1;
	u32 pwt:1;
	u32 pcd:1;
	u32 acessado:1;
	u32 avl:1;
	u32 ps:1;
	u32 global:1;
	u32 avail:3;
	u32 base:20;
} __attribute__((packed));

struct item_tabela {
	u32 presente:1;
	u32 rw:1;
	u32 us:1;
	u32 pwt:1;
	u32 pcd:1;
	u32 acessado:1;
	u32 sujo:1;
	u32 pat:1;
	u32 global:1;
	u32 avail:3;
	u32 base:20;
} __attribute__((packed));

#define N_ENTRADAS_DIRETORIO (TAM_PAGINA / sizeof (struct item_diretorio))
#define N_ENTRADAS_TABELA (TAM_PAGINA / sizeof (struct item_tabela))

/*
 * Diretorio para as tabelas de pagina
 */
static struct item_diretorio diretorio[N_ENTRADAS_DIRETORIO]
	__attribute__ ((aligned(4096)));
static u32 endereco_fisico_dir;

static int (*aloc)(u32 bytes, u32 *end);

/* Esta função recebe por parâmetro a estrutura do multiboot e finaliza com o
 * diretório definitivo alocado e funcionando.
 */
static void inicializa_diretorio_ia32(void)
{
	u32 i;

	endereco_fisico_dir = __virtual_real_precoce(diretorio);

	/* zera diretorio */
	memset(diretorio, 0, sizeof diretorio);

	/* privilegios de read/write e user/supervisor devem ser feitos na
	 * tabela de páginas.
	 */
	for (i = 0; i < N_ENTRADAS_DIRETORIO; ++i) {
		diretorio[i].rw = 1;
		diretorio[i].us = 1;
	}

	/* Tabelas de páginas mapeadas de 0xffc00000 até 0xffffffff. */
	diretorio[1023].us = 0;
	diretorio[1023].base = endereco_fisico_dir>>12;
	diretorio[1023].presente = 1;
}

/*
 * Efetiva qualquer alteração que tenha sido feita na tabela de páginas ao se
 * chamar as funções de relacionamento de páginas.
 */
static inline void flush_ia32(void)
{
	asm volatile (
		"mov %%eax, %%cr3;"
		:
		: "a" (endereco_fisico_dir)
	);
}

static inline void* pega_tp(u32 entrada_diretorio)
{
	return (void*)((entrada_diretorio<<12) | 0xffc00000);
}

static inline int cria_diretorio(u32 id)
{
	u32 tmp;
	int erro;
	struct tabela_item *tabela;

	erro = aloc(4096, &tmp);
	if (erro)
		return FALTA_MEMORIA;
	diretorio[id].base = tmp>>12;
	diretorio[id].presente = 1;

	tabela = pega_tp(id);

	memset(tabela, 0, 4096);

	return 0;
}

/*
 * adiciona_ia32 - relaciona uma pagina virtual a um frame físico.
 * @p_virtual - 20 bits que representam a página em que o endereço físico será
 *              mapeado.
 * @f_fisico  - 20 bits que representam o frame físico ao qual a página está
 *              sendo mapeada.
 * @flags     - flags que controlam as propriedades da página.
 */
static int adiciona_ia32(u32 p_virtual, u32 f_fisico, u32 flags)
{
	struct item_tabela *tabela;
	u32 id_tabela = p_virtual >> 10;
	u32 id_pagina = p_virtual & 0x000003ff;

	if (!diretorio[id_tabela].presente) {
		int erro;
		erro = cria_diretorio(id_tabela);
		if (erro) return erro;
	}

	tabela = pega_tp(id_tabela);

	if (tabela[id_pagina].presente && !(flags & FORCE))
		return PAGINA_EXISTENTE;

	memset(&(tabela[id_pagina]), 0, sizeof (struct item_tabela));

	if (!(flags & KERN))
		tabela[id_pagina].us = 1;
	if (flags & RW)
		tabela[id_pagina].rw = 1;
	if (flags & WRITE_THROUGH)
		tabela[id_pagina].pwt = 1;
	if (flags & DESATIVA_CACHE)
		tabela[id_pagina].pcd = 1;
	tabela[id_pagina].base = f_fisico;
	tabela[id_pagina].presente = 1;

	return 0;
}

/*
 * remove_ia32 - remove uma página virtual do sistema.
 * @p_virtual - 20 bits que representam a página em que o endereço físico será
 *              mapeado.
 */
static int remove_ia32(u32 p_virtual)
{
	struct item_tabela *tabela;
	u32 id_tabela = p_virtual >> 10;
	u32 id_pagina = p_virtual & 0x000003ff;
	
	if (!diretorio[id_tabela].presente)
		return PAGINA_INEXISTENTE;
	tabela = pega_tp(id_tabela);
	if (!tabela[id_pagina].presente)
		return PAGINA_INEXISTENTE;
	
	memset(&(tabela[id_pagina]), 0, sizeof (struct item_tabela));

	return 0;
}

/*
 * virtual_fisico_ia32 - transforma um endereço virtual em um endereço fisico.
 * @end_virtual - os 32 bits de endereço real que se quer transformar em físico.
 * @end_real    - endereço físico resultante da transformação.
 */
static int virtual_real_ia32(void *end_virtual, u32 *end_real)
{
	struct item_tabela *tabela;
	u32 virtual = (u32) end_virtual;
	u32 id_tabela = virtual >> 22;
	u32 id_pagina = (virtual>>12) & 0x000003ff;

	if (!diretorio[id_tabela].presente)
		return PAGINA_INEXISTENTE;
	tabela = pega_tp(id_tabela);
	if (!tabela[id_pagina].presente)
		return PAGINA_INEXISTENTE;

	*end_real = (tabela[id_pagina].base<<12) | (virtual&0x000003ff);

	return 0;
}

static void use_alocador_ia32(int (*f)(u32 bytes, u32 *end))
{
	aloc = f;
}

static struct paginacao pag = {
	.adiciona = adiciona_ia32,
	.remove = remove_ia32,
	.flush = flush_ia32,
	.virtual_real = virtual_real_ia32,
	.use_alocador = use_alocador_ia32,
	.inicio_reservado = 0xffc00000,
	.fim_reservado = 0xffffffff,
};

/*
 * Encontra o final da memória inferior (low memory). Depois dela existem
 * endereços para acessar a rom da BIOS e a placa de vídeo. E depois disso (a
 * partir de INICIO_HIGHMEM existe o kernel que vai até a variável
 * final_estatico.
 */
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

static void mapeamento_inicial(struct multiboot_info *mbi)
{
	u32 base = HIGH_HALF >> 12;
	u32 i, inicio_utilizado;

	inicio_utilizado = fim_lowmem(mbi);

	/* Paginas referentes ao buraco no meio da memória */
	for (i = inicio_utilizado>>12; i < (INICIO_HIGHMEM>>12); ++i) {
		struct item_tabela *tabela;
		u32 id_tabela = (i+base) >> 10;
		u32 id_pagina = (i+base) & 0x000003ff;

		if (!diretorio[id_tabela].presente)
			cria_diretorio(id_tabela);

		tabela = (void*)(diretorio[id_tabela].base << 12);

		memset(&(tabela[id_pagina]), 0, sizeof (struct item_tabela));

		tabela[id_pagina].rw = 1;
		tabela[id_pagina].pcd = 1;
		tabela[id_pagina].base = i;
		tabela[id_pagina].presente = 1;
	}

	/* Paginas referentes ao kernel */
	for (i = INICIO_HIGHMEM>>12; i < (fim_kernel>>12); ++i) {
		struct item_tabela *tabela;
		u32 id_tabela = (i+base) >> 10;
		u32 id_pagina = (i+base) & 0x000003ff;

		if (!diretorio[id_tabela].presente)
			cria_diretorio(id_tabela);

		tabela = (void*)(diretorio[id_tabela].base << 12);

		memset(&(tabela[id_pagina]), 0, sizeof (struct item_tabela));

		tabela[id_pagina].rw = 1;
		tabela[id_pagina].base = i;
		tabela[id_pagina].presente = 1;
	}
	flush_ia32();
}

void* inicializa_paginacao_ia32(int (*f)(u32 bytes, u32 *end),
                                struct multiboot_info *mbi)
{
	aloc = f;

	inicializa_diretorio_ia32();

	mapeamento_inicial(mbi);

	return &pag;
}
