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

#include <mm/mm.h>
#include <multiboot.h>

/* Final do kernel contando apenas o código e conteúdo estático (colocado na
 * tabela de simbolos pelo linker).
 */
extern int final_estatico;

int inicializa_mm(struct multiboot_info *mbi)
{
	int erro;

	if (!mbi->flags.mmap)
		return -1;

	u32 final = virtual_real_boot(&final_estatico);

	erro = inicializa_alocacao_fisica(mbi, final);
	if (erro) return erro;
	erro = inicializa_paginacao(mbi);
	if (erro) return erro;

	return 0;
}
