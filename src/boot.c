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

#include <tipos.h>
#include <multiboot.h>
#include <klog.h>
#include <vm.h>

/* Função inicial do kernel. É aqui que tudo começa :-). */
void kmain(struct multiboot_info *mbi)
{
	if (inicia_vm(mbi) == SUCESSO) {
		klog(NOTA, "Sistema de Controle de Memoria incializado.\n");
	} else {
		klog(ERRO, "Sistema de controle de Memoria falhou.\n");
		return;
	}

	while(1);
}
