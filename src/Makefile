DEPDIR = .deps

CC = gcc

# A / no final do diretorio é fundamental
OBJDIR = obj/

BIN = papagaio

A_SRCS = papagaio.s

C_SRCS = boot.c\
         console.c\
         string.c\
         klog.c
# mm/
C_SRCS += alocacao_fisica.c\
          paginacao_ia32.c\
          paginacao.c\
	  malloc.c\
          mm.c

OBJ = $(addprefix $(OBJDIR),$(C_SRCS:.c=.o))\
      $(addprefix $(OBJDIR),$(A_SRCS:.s=.o))

CPPFLAGS = -Iinclude/
CFLAGS = -O2 -W -Wall -m32 -fno-strict-aliasing -fno-builtin\
-nostdlib -static -fno-stack-protector

MAKEDEPEND = gcc -M

.PHONY: all clean
all: papagaio
clean:
	rm -rf $(OBJDIR) $(BIN) $(DEPDIR)


papagaio: $(OBJ) papagaio.ld
	ld -Tpapagaio.ld -melf_i386 $(OBJ) -opapagaio

obj/papagaio.o: papagaio.s
	nasm -felf32 papagaio.s -o$(OBJDIR)/papagaio.o

obj/%.o : %.c
	@mkdir -p $(OBJDIR) $(DEPDIR)
	$(COMPILE.c) -MMD -o $(OBJDIR)/$(@F) $<
	@mv $(OBJDIR)/$(*F).d $(DEPDIR)/
	@cp $(DEPDIR)/$(*F).d $(DEPDIR)/$(*F).P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	    -e '/^$$/ d' -e 's/$$/ :/' < $(DEPDIR)/$(*F).d >> $(DEPDIR)/$(*F).P; \
	rm -f $(DEPDIR)/$(*F).d

obj/%.o : mm/%.c
	@mkdir -p $(OBJDIR) $(DEPDIR)
	$(COMPILE.c) -MMD -o $(OBJDIR)/$(@F) $<
	@mv $(OBJDIR)/$(*F).d $(DEPDIR)/
	@cp $(DEPDIR)/$(*F).d $(DEPDIR)/$(*F).P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	    -e '/^$$/ d' -e 's/$$/ :/' < $(DEPDIR)/$(*F).d >> $(DEPDIR)/$(*F).P; \
	rm -f $(DEPDIR)/$(*F).d


-include $(C_SRCS:%.c=$(DEPDIR)/%.P)
