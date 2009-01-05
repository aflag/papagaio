#ifndef __VM_H
#define __VM_H
/* public domain */

struct item_diretorio {
	u32 presente:1;
	u32 rw:1;
	u32 us:1;
	u32 pwt:1;
	u32 pcd:1;
	u32 acessado:1;
	u32 avl:1;
	u32 ps:1;
	u32 global:1;
	u32 avail:3;
	u32 base:20;
} __attribute__((packed));

struct item_tabela {
	u32 presente:1;
	u32 rw:1;
	u32 us:1;
	u32 pwt:1;
	u32 pcd:1;
	u32 acessado:1;
	u32 sujo:1;
	u32 pat:1;
	u32 global:1;
	u32 avail:3;
	u32 base:20;
} __attribute__((packed));

int inicia_vm(struct multiboot_info *mbi);

#define TAM_PAGINA (4096)
#define N_ENTRADAS_DIRETORIO (TAM_PAGINA / sizeof (struct item_diretorio))
#define N_ENTRADAS_TABELA (TAM_PAGINA / sizeof (struct item_tabela))
#define HIGH_HALF (0xc0000000)
#define INICIO_HIGHMEM (0x100000)

#endif
