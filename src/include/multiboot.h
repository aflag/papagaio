#ifndef __MULTIBOOT_H
#define __MULTIBOOT_H
/* public domain */

#include <tipos.h>

struct mmap_record {
	u32 size;
	u32 base_low;
	u32 base_high;
	u32 length_low;
	u32 length_high;
	u32 type;
};

struct flag {
	u32 mem:1;
	u32 boot_device:1;
	u32 cmdline:1;
	u32 mods:1;
	u32 a_out:1;
	u32 elf:1;
	u32 mmap:1;
	u32 drives:1;
	u32 config_table:1;
	u32 bootloader_name:1;
	u32 apm_table:1;
	u32 graphics:1;
	u32 reserved:20;
} __attribute__((packed));

struct multiboot_info {
	struct flag flags;

	u32 mem_lower;
	u32 mem_upper;
	u32 boot_device;
	u32 cmdline;
	u32 mods_count;
	u32 mods_addr;

	/* XXX: O valor a seguir é apenas um place-holder, caso passe a ser
	 * utilizado deve-se criar uma struct só para ele.
	 */
	u32 file_hdr[4];

	u32 mmap_length;
	struct mmap_record *mmap;
};

#endif
