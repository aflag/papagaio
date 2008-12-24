OBJS = papagaio.o boot.o console.o string.o klog.o
CFLAGS = -m32 -Iinclude/ -fno-builtin -nostdinc -nostdlib -static

papagaio: $(OBJS)
	ld -Tpapagaio.ld -melf_i386 $(OBJS) -opapagaio

clean:
	rm -f $(OBJS)

papagaio.o: papagaio.s
	nasm -felf32 papagaio.s

boot.o: boot.c include/klog.h
console.o: console.c include/console.h
string.o: string.c
klog.o: klog.c include/console.h include/erros.h include/klog.h
