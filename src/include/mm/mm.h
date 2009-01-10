#ifndef __VM_H
#define __VM_H
/* public domain */

#include <mm/alocacao_fisica.h>
#include <mm/paginacao.h>
#include <multiboot.h>

int inicializa_mm(struct multiboot_info *mbi);

#endif
