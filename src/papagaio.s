[BITS 32]
[GLOBAL start]

multiboot:
	magic    dd  0x1BADB002
	flags    dd  0x0
	checksum dd  -(0x1BADB002)


start:
	jmp $
