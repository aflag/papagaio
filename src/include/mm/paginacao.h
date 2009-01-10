#ifndef __PAGINACAO_H
#define __PAGINACAO_H
/* public domain */

#include <tipos.h>
/* transforma endereços virtuais em físicos antes da paginação ter sido
 * propriamente inicializada
 */
inline u32 __virtual_real_precoce(void *ptr);

#define TAM_PAGINA 4096
#define HIGH_HALF 0xc0000000
#define INICIO_HIGHMEM 0x100000

/* flags para a criacao de páginas */
#define KERN           0x00000001
#define FORCE          0x00000002
#define RW             0x00000004
#define WRITE_THROUGH  0x00000008
#define DESATIVA_CACHE 0x00000010
#define GLOBAL         0x00000020
#define NAO_EXEC       0x00000040
#define INICIALIZACAO  0x00000080

enum {
	FALTA_MEMORIA = 1,
	PAGINA_EXISTENTE,
	PAGINA_INEXISTENTE,
};

struct paginacao {
	int (*adiciona)(u32 p_virtual, u32 f_fisico, u32 flags);
	int (*remove)(u32 p_virtual);
	void (*flush)(void);
	int (*virtual_real)(void *end_virtual, u32 *end_real);
	void (*use_alocador)(int (*aloc)(u32 bytes, u32 *end));
	u32 inicio_reservado;
	u32 fim_reservado;
};

extern struct paginacao *tab_paginas;

int inicializa_paginacao(struct multiboot_info *mbi);

#endif
