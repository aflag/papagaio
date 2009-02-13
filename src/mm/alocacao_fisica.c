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
#include <multiboot.h>
#include <klog.h>
#include <mm/mm.h>
#include <queue.h>

/* Final do kernel com o código, conteúdo estático e mais a tabela de frames
 * livres.
 */
u32 fim_kernel;

struct frame *frames;

static LIST_HEAD(, frame) frames_livres = LIST_HEAD_INITIALIZER(frames_livres);

static u32 ultimo_frame;

/*
 * I n t e r f a c e
 */
/*
 * aloca_fis - aloca um frame da memória física.
 * @endereco - endereco do espaco alocado.
 */
struct frame* aloca_fis(void)
{
	if (LIST_EMPTY(&frames_livres))
		return 0;
	else {
		struct frame *f;

		f = LIST_FIRST(&frames_livres);
		LIST_REMOVE(f, frames);

		return f;
	}
}

/*
 * libera_fis - libera memória física alocada por aloca_fis.
 * @end - endereço da memória física a ser desalocada.
 */
void libera_fis(struct frame *f)
{
	if (f < frames || f >= (frames+ultimo_frame))
		klog(ERRO, "libera_fis: Frame invalido: %p [%p, %p]\n", f,
		     frames, frames + ultimo_frame);
	if (f->endereco & 0x00000fff)
		klog(ERRO, "Tentando liberar endereco invalido: %x\n",
		     f->endereco);
	else
		LIST_INSERT_HEAD(&frames_livres, f, frames);
}

/*
 * aloca_boot - aloca bytes da memória física. Esta função deve ser utilizada
 *              na inicialização da paginação. Ela deve retornar um endereço
 *              menor que 4MB ou FALTA_ESPACO quando não existir tal endereço
 *              livre. Esta função depende da função cria_lista_livres
 *              começar a empilhar os endereços em ordem decrescente.
 * @endereco - endereco do espaco alocado.
 */
struct frame* aloca_boot(void)
{
	if (!LIST_EMPTY(&frames_livres)
	    && LIST_FIRST(&frames_livres)->endereco > 0x1400000)
		return 0;
	else
		return aloca_fis();
}

/*
 * F u n ç õ e s  d e  i n i c i a l i z a ç ã o
 */
/*
 * Calcula o número de frames disponíveis na memória física (sem contar qualquer
 * coisa antes do espaço de alocação do kernel).
 */
static __inline__ u32 conta_frames(struct multiboot_info *mbi)
{
	struct mmap_record *minfo = mbi->mmap;
	u32 i; 
	u32 total = 0;
	u32 tam = mbi->mmap_length / (sizeof (struct mmap_record));

	for (i = 0; i < tam; ++i)
		/* tipo 1 = memoria utilizavel */
		if (minfo[i].type == 1) {
			u32 base = minfo[i].base_low, tam = minfo[i].length_low;
			u32 ultimo = (base + tam) >> 12;
			u32 primeiro;
			if (base > fim_kernel)
				primeiro = fim_kernel >> 12;
			else
				primeiro = base >> 12;

			total += ultimo - primeiro;
		}

	return total;
}

/*
 * aloca memoria a partir do final do kernel. O endereço virtual é obtido
 * somando-se HIGH_HALF. Esta função é para uso apenas durante a inicialização
 * do sistema.
 */
static __inline__ void* __aloca(u32 bytes)
{
	u32 tmp = fim_kernel;

	fim_kernel += bytes;
	fim_kernel = (fim_kernel&0xfffff000) + 0x1000;

	return (void*) (tmp + INICIO_VIRTUAL);
}

/* 
 * Aloca uma array com os endereços de todas os frames disponíveis. Esses serão
 * todos frames que poderão ser utilizados para alocação de memória dinâmica.
 */
static void cria_frame_pool(struct multiboot_info *mbi)
{
	struct mmap_record *minfo = mbi->mmap;
	int tam_mmap = mbi->mmap_length / (sizeof *minfo);
	int i;
	u32 total_frames;

	total_frames = conta_frames(mbi);
	frames = __aloca(total_frames * (sizeof *frames));

	ultimo_frame = 0;
	for (i = tam_mmap-1; i >= 0; --i)
		if (minfo[i].type == 1) {
			u32 base = minfo[i].base_low, tam = minfo[i].length_low;
			u32 primeiro = base >> 12;
			u32 ultimo = (base + tam) >> 12;
			u32 frame;

			if (base & 0x00000fff) {
				klog(ALERTA, "Memoria nao alinhada:\n");
				klog(ALERTA, "  %x\n", base);
			}

			frame = ultimo;
			while ((frame >= primeiro)
			       && (frame >= (fim_kernel>>12)))
				frames[ultimo_frame++].endereco = frame-- << 12;
		}
}

static void cria_lista_vazios(void)
{
	u32 i;

	for (i = 0; i < ultimo_frame; ++i)
		LIST_INSERT_HEAD(&frames_livres, &frames[i], frames);
}

/*
 * inicializa_alocacao_fisica - inicializa os mecanismos para alocação de
 *                              memória física.
 * @mbi - estrutura multiboot que o grub ou outro bootloader deve ter retornado.
 * @final - endereço do final do kernel no momento da chamada.
 */
int inicializa_alocacao_fisica(struct multiboot_info *mbi, u32 final)
{
	fim_kernel = final;

	cria_frame_pool(mbi);

	cria_lista_vazios();

	return 0;
}
