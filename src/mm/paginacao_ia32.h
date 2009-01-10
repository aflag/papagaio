#ifndef __PAGINACAO_IA32_H
#define __PAGINACAO_IA32_H
/* public domain */

void* inicializa_paginacao_ia32(int (*aloc)(u32 bytes, u32 *end),
                                struct multiboot_info *mbi);
#endif
