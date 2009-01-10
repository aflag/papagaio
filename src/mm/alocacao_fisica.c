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

/* Final do kernel com o código, conteúdo estático e mais a tabela de frames
 * livres.
 */
u32 fim_kernel;

static u32 *frames_livres;
static u32 proximo_frame;

/*
 * F u n ç õ e s  A u x i l i a r e s.
 */
/*
 * Determina se a lista de frames está vazia (0) ou ainda há frames (>0).
 */
static __inline__ int existe_frame(void)
{
	return proximo_frame;
}

/*
 * Retira um frame da lista.
 */
static __inline__ int pop_frame(u32 *frame)
{
	if (!existe_frame())
		return TODOS_FRAMES_OCUPADOS;

	*frame = frames_livres[--proximo_frame];

	return 0;
}

/*
 * Adiciona um novo frame à lista.
 */
static __inline__ void adiciona_frame(u32 frame)
{
	frames_livres[proximo_frame++] = frame;
}

/*
 * I n t e r f a c e
 */
/*
 * aloca_fis - aloca bytes na memória física.
 * @bytes    - quantidade de memória a ser alocada.
 * @endereco - endereco do espaco alocado.
 */
int aloca_fis(u32 bytes, u32 *endereco)
{
	int erro;

	if (bytes > TAM_FRAME)
		return FALTA_ESPACO;

	erro = pop_frame(endereco);
	*endereco <<= 12;

	return erro;
}

/*
 * aloca_inicial - aloca bytes da memória física. Esta função deve ser utilizada
 *                 na inicialização da paginação. Ela deve retornar um endereço
 *                 menor que 4MB ou FALTA_ESPACO quando não existir tal endereço
 *                 livre. Esta função depende da função cria_lista_livres
 *                 começar a empilhar os endereços em ordem decrescente.
 * @bytes    - quantidade de memória a ser alocada.
 * @endereco - endereco do espaco alocado.
 */
int aloca_inicial(u32 bytes, u32 *endereco)
{
	if (existe_frame() && (frames_livres[proximo_frame-1] > (0x400000>>12)))
		return FALTA_ESPACO;
	return aloca_fis(bytes, endereco);
}

/*
 * libera_fis - libera memória física alocada por aloca_fis.
 * @end - endereço da memória física a ser desalocada.
 */
void libera_fis(u32 end)
{
	if (end & 0x00000fff)
		klog(ERRO, "Tentando liberar endereco invalido: %x\n", end);
	else
		adiciona_frame(end>>12);
}

/*
 * F u n ç õ e s  d e  i n i c i a l i z a ç ã o
 */
/*
 * Calcula o número de frames disponíveis na memória física.
 */
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

	return (void*) (tmp + HIGH_HALF);
}

/* 
 * Aloca depois do final do kernel uma array com os endereços de todas os frames
 * disponíveis. Quando requisitado o sistema irá retornar o primeiro frame
 * vázio.
 */
static void cria_lista_livres(struct multiboot_info *mbi)
{
	struct mmap_record *minfo = mbi->mmap;
	int tam_mmap = mbi->mmap_length / (sizeof (struct mmap_record));
	int i;
	u32 total_frames;

	total_frames = conta_frames(mbi);
	frames_livres = __aloca(total_frames * sizeof (u32));

	proximo_frame = 0;
	for (i = tam_mmap-1; i >= 0; --i)
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

	cria_lista_livres(mbi);

	return 0;
}
