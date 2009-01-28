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

[global tpag_inicial]
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

; -*- Opções do registrador CR0 -*-
CR0_PG equ (1<<31)
CR0_CD equ (1<<30)
CR0_NW equ (1<<29)
CR0_AM equ (1<<18)
CR0_WP equ (1<<16)
CR0_NE equ (1<<5)
CR0_ET equ (1<<4)
CR0_TS equ (1<<3)
CR0_EM equ (1<<2)
CR0_MP equ (1<<1)
CR0_PE equ (1<<0)

CR0_FLAGS equ CR0_PG
; -*- -*-

; -*- Opções do registrador CR4 -*-
CR4_OSXSAVE equ (1<<18)
CR4_SMXE equ (1<<14)
CR4_VMXE equ (1<<13)
CR4_OSXMMEXCPT equ (1<<10)
CR4_OSFXSR equ (1<<9)
CR4_PCE equ (1<<8)
CR4_PGE equ (1<<7)
CR4_MCE equ (1<<6)
CR4_PAE equ (1<<5)
CR4_PSE equ (1<<4)
CR4_DE equ (1<<3)
CR4_TSD equ (1<<2)
CR4_PVI equ (1<<1)
CR4_VME equ (1<<1)

CR4_FLAGS equ (CR4_PSE | CR4_PGE)
; -*- -*-

inicio:
	cli	;

	mov edx, tpag_inicial
	and edx, 0xffc00000
	or edx, 0x9b
	mov [tpag_inicial+4092], edx

	mov eax, tpag_inicial	; O diretório de páginas encontra-se no endereço
	mov cr3, eax		; físico tpag_inicial.

	mov ecx, cr4		; Liga o bit no registrador cr4 que faz
	or ecx, CR4_FLAGS	; com que a paginação seja feita com
	mov cr4, ecx		; páginas de 4MB.

	mov eax, cr0		; Liga paginação no registrador cr0.
	or eax, CR0_FLAGS	;
	mov cr0, eax		;

	mov esp, stack + 0x4000	;

	push ebx	; Carrega endereço da estrutura do Multiboot

	call kmain	;
	hlt		;
	jmp $-1		;


; -*- Macros para tabela de páginas -*-
	; tpag_entrada - Gera uma entrada para a pagina especificada, dado o
	;                numero da página. Bits ligados:
	;                    * Present (bit 0)
	;                    * R/W (bit 1)
	;                    * PWT (bit 3)
	;                    * PCD (bit 4)
	;                    * PS (bit 7)
%define tpag_entrada(num_pag) (((num_pag)<<22) | 0x9b)

	; pula_entradas - Retorna a distância entre a posição da entrada
	;                 desejada e a posição da entrada atual.
%define pula_entradas(pos_desejada, pos_atual) ((pos_desejada) - (pos_atual))

	; endereco_entrada - Transforma um endereço na posição da entrada na
	;                    tabela de páginas.
%define pagina(endereco) (endereco>>22)

KERNEL_FISICO equ pagina(0x01000000)
INICIO_VIRTUAL equ pagina(0xc0000000)
; -*- -*-

; Tabela de pagina inicial:
; 0x00000000 - 0x01400000 físico = 0x00000000 - 0x01400000 virtual
; 0x00000000 - 0x01400000 físico = 0xc0000000 - 0xc1400000 virtual
align 4096
tpag_inicial:
	dd tpag_entrada(0)
	dd tpag_entrada(1)
	dd tpag_entrada(2)
	dd tpag_entrada(3)
	dd tpag_entrada(4)

	times pula_entradas(INICIO_VIRTUAL, KERNEL_FISICO+1) dd 0
	dd tpag_entrada(0)
	dd tpag_entrada(1)
	dd tpag_entrada(2)
	dd tpag_entrada(3)
	dd tpag_entrada(4)

	times pula_entradas(pagina(0xffffffff), INICIO_VIRTUAL+5) dd 0

[section .bss]
align 32
stack:
	resb 0x4000
