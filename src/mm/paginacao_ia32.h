#ifndef __PAGINACAO_IA32_H
#define __PAGINACAO_IA32_H
/* public domain */

void* inicializa_paginacao_ia32(int (*aloc)(u32 bytes, u32 *end),
                                struct multiboot_info *mbi);
int virtual_fisico_boot_ia32(void *end_virtual, u32 *end_real);

#define num_pagina(endereco) ((endereco) >> 12)
#define indice_diretorio(pagina) ((pagina) >> 10)
#define indice_tabela(pagina) ((pagina) & 0x3ff)

#endif
