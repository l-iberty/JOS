KERNEL_SRCDIR := kernel
LIB_SRCDIR := lib

KERNEL_OBJS := $(OBJDIR)/kernel/entry.o $(OBJDIR)/kernel/lib.o $(OBJDIR)/kernel/console.o
KERNEL_OBJS += $(OBJDIR)/kernel/printf.o $(OBJDIR)/kernel/init.o $(OBJDIR)/kernel/entrypgdir.o

LIB_OBJS := $(OBJDIR)/lib/vsprintf.o $(OBJDIR)/lib/string.o

all: $(OBJDIR)/jos.img

$(OBJDIR)/kernel/entry.o: $(KERNEL_SRCDIR)/entry.asm
	mkdir -p $(@D)
	$(AS) $(ASFLAGS) $< -o $@

$(OBJDIR)/kernel/lib.o: $(KERNEL_SRCDIR)/lib.asm
	$(AS) $(ASFLAGS) $< -o $@

$(OBJDIR)/kernel/console.o: $(KERNEL_SRCDIR)/console.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/kernel/printf.o: $(KERNEL_SRCDIR)/printf.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/kernel/init.o: $(KERNEL_SRCDIR)/init.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/kernel/entrypgdir.o: $(KERNEL_SRCDIR)/entrypgdir.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/lib/vsprintf.o: $(LIB_SRCDIR)/vsprintf.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/lib/string.o: $(LIB_SRCDIR)/string.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/kernel/kernel: $(KERNEL_OBJS) $(LIB_OBJS)
	$(LD) $(LDFLAGS) -T $(KERNEL_SRCDIR)/kernel.ld $^ -o $@
	$(OBJDUMP) -S $@ >$@.objdump

$(OBJDIR)/jos.img: $(OBJDIR)/kernel/kernel $(OBJDIR)/boot/boot
	dd if=/dev/zero of=$@~ count=10000 2>/dev/null
	dd if=$(OBJDIR)/boot/boot of=$@~ conv=notrunc 2>/dev/null
	dd if=$(OBJDIR)/kernel/kernel of=$@~ seek=1 conv=notrunc 2>/dev/null
	mv $@~ $@