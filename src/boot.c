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
#include <bit.h>
#include <multiboot.h>
#include <klog.h>

extern char final_kernel;

/* Função inicial do kernel. É aqui que tudo começa :-). */
void kmain(struct multiboot_info *mbi)
{
	u32 flags = mbi->flags;

	debug( "flags: %x\n", flags);
	if (is_set(flags, 6)) {
		struct mmap_record *x = mbi->mmap;
		u32 i;
		u32 total = mbi->mmap_length / (sizeof (struct mmap_record));
		for (i = 0; i < total; ++i) {
			debug("type: %x, base: %x, length: %x\n",
			      x[i].type, x[i].base_low, x[i].length_low);
		}
	}
}
