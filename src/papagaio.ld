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

ENTRY(inicio)
SECTIONS
{
	. = 0x1000000;

	inicio_estatico = .;

	.comeco :
	{
		*(.comeco)
		. = ALIGN(4096);
	}

	. += 0xc0000000;

	.text : AT (ADDR(.text) - 0xc0000000)
	{
		*(.text)
		. = ALIGN(4096);
	}

	.data : AT (ADDR(.data) - 0xc0000000)
	{
		*(.data)
		*(.rodata*)
		. = ALIGN(4096);
	}

	.bss : AT (ADDR(.bss) - 0xc0000000)
	{
		*(.bss)
		. = ALIGN(4096);
	}

	final_estatico = .;
}
