OBJS = papagaio.o boot.o console.o string.o klog.o vm.o
CFLAGS = -O2 -W -Wall -m32 -Iinclude/ -fno-strict-aliasing -fno-builtin\
-nostdlib -static -fno-stack-protector

papagaio: $(OBJS) papagaio.ld
	ld -Tpapagaio.ld -melf_i386 $(OBJS) -opapagaio

clean:
	rm -f $(OBJS)

papagaio.o: papagaio.s
	nasm -felf32 papagaio.s

boot.o: boot.c include/tipos.h include/bit.h include/multiboot.h include/klog.h\
include/vm.h
console.o: console.c include/console.h
string.o: string.c
klog.o: klog.c include/console.h include/erros.h include/klog.h
vm.o: vm.c include/vm.h
