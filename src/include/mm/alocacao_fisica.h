#ifndef __ALOCACAO_FISICA_H
#define __ALOCACAO_FISICA_H
/* public domain */

#include <multiboot.h>
#include <tipos.h>
#include <queue.h>

extern u32 fim_kernel;

struct frame {
	LIST_ENTRY(frame) frames;
	u32 endereco;
	u16 offset;
	u16 contador;
	void *meta_pagina;
};

struct frame* aloca_fis(void);
struct frame* aloca_boot(void);
void libera_fis(struct frame *f);

int inicializa_alocacao_fisica(struct multiboot_info *mbi, u32 final);

enum {
	FALTA_ESPACO = 1,
};

#define TAM_FRAME 4096

#endif
