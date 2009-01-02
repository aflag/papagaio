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

#include <stdarg.h>

#include <console.h>
#include <erros.h>
#include <klog.h>

/* Esta função serve para logar as atividades do kernel, debugar novos sistemas,
 * etc. O primeiro parâmetro escolhe a prioridade do log.
 *
 * Se a função for bem sucedida ela retorna 0. Caso contrário ela retorna um dos
 * erros em erros.h.
 */
int klog(int prioridade, const char *s, ...)
{
	va_list ap;

	va_start(ap, s);

	switch (prioridade) {
		case IMPORTANTE:
		case AVISO:
		case DEBUG:
			vimprime(s, ap);
			return 0;
		default:
			return E_NDEF;
	}

	va_end(ap);
}
