#ifndef __ALOCACAO_FISICA_H
#define __ALOCACAO_FISICA_H
/* public domain */

#include <multiboot.h>
#include <tipos.h>

extern u32 fim_kernel;

int aloca_fis(u32 bytes, u32 *endereco);
int aloca_boot(u32 bytes, u32 *endereco);
void libera_fis(u32 end);

int inicializa_alocacao_fisica(struct multiboot_info *mbi, u32 final);

enum {
	TODOS_FRAMES_OCUPADOS = 1,
	FALTA_ESPACO
};

#define TAM_FRAME 4096

#endif
