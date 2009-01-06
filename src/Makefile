MAKEDEPEND = gcc -M

DEPDIR = .deps

BIN = papagaio

SRCS = boot.c\
       console.c\
       string.c\
       klog.c\
       vm.c
OBJ = $(SRCS:.c=.o) papagaio.o

CPPFLAGS = -Iinclude/
CFLAGS = -O2 -W -Wall -m32 -fno-strict-aliasing -fno-builtin\
-nostdlib -static -fno-stack-protector

.PHONY: all clean
all: papagaio
clean:
	rm -f $(OBJ)


papagaio: $(OBJ) papagaio.ld
	ld -Tpapagaio.ld -melf_i386 $(OBJ) -opapagaio

papagaio.o: papagaio.s
	nasm -felf32 papagaio.s

%.o : %.c
	$(COMPILE.c) -MMD -o $@ $<
	@mv $(*F).d $(DEPDIR)/
	@cp $(DEPDIR)/$(*F).d $(DEPDIR)/$(*F).P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	    -e '/^$$/ d' -e 's/$$/ :/' < $(DEPDIR)/$(*F).d >> $(DEPDIR)/$(*F).P; \
	rm -f $(DEPDIR)/$(*F).d

-include $(SRCS:%.c=$(DEPDIR)/%.P)
