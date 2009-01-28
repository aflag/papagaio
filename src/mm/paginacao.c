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
#include <string.h>
#include <mm/mm.h>

#include "paginacao_ia32.h"

struct paginacao *tab_paginas;

void paginacao_carrega_funcoes_boot(void)
{
	virtual_fisico_boot = virtual_fisico_boot_ia32;
}

int inicializa_paginacao(struct multiboot_info *mbi)
{
	tab_paginas = inicializa_paginacao_ia32(aloca_boot, mbi);
	tab_paginas->use_alocador(aloca_fis);

	return 0;
}
