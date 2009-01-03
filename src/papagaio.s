; Copyright (c) 2008, 2009, Rafael Cunha de Almeida <almeidaraf@gmail.com>
;
; Permission to use, copy, modify, and/or distribute this software for any
; purpose with or without fee is hereby granted, provided that the above
; copyright notice and this permission notice appear in all copies.
;
; THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
; WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
; MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
; ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
; WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
; ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
; OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

[bits 32]

[extern kmain]
[extern final_kernel]
[global inicio]

[section .comeco]

; -*- COMUNICAÇÃO COM O BOOTLOADER -*-
MB_MAGIC equ 0x1BADB002
MB_ALIGNED_FLAG equ (1 << 0)
MB_MEMINFO_FLAG equ (1 << 1)
MB_FLAGS equ (MB_ALIGNED_FLAG | MB_MEMINFO_FLAG)

	dd MB_MAGIC
	dd MB_FLAGS
	dd -(MB_MAGIC + MB_FLAGS)
; -*- FIM DA COMUNICAÇÃO COM BOOTLOADER -*-

inicio:
	; Preenche o diretório de páginas com a entrada 0x19b. Assim todas
	; entradas apontam para a página 0. Bits ligados:
	;	* Present (bit 0)
	;	* R/W (bit 1)
	;	* PWT (bit 3)
	;	* PCD (bit 4)
	;	* PS (bit 7)
	;	* Global (bit 8)
	mov ecx, 0
preenche_tabela:
	mov eax, ecx
	shl eax, 2
	mov [eax], dword 0x19b
	cmp ecx, 1024
	inc ecx
	jb preenche_tabela

	; O diretório de páginas encontra-se no endereço físico 0x0.
	mov eax, 0
	mov cr3, eax

	; Liga o bit no registrador cr4 que faz com que a paginação seja feita
	; com páginas de 4MB.
	mov edx, cr4
	or edx, 1<<4
	mov cr4, edx

	; Liga paginação no registrador cr0.
	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax

	; Carrega endereço da estrutura do Multiboot
	push ebx

	call kmain
	hlt
	jmp $-1
