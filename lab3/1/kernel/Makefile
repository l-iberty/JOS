KERNEL_SRCDIR := kernel
LIB_SRCDIR := lib

KERNEL_OBJS := $(OBJDIR)/kernel/entry.o $(OBJDIR)/kernel/lib.o $(OBJDIR)/kernel/console.o
KERNEL_OBJS += $(OBJDIR)/kernel/init.o $(OBJDIR)/kernel/entrypgdir.o $(OBJDIR)/kernel/monitor.o
KERNEL_OBJS += $(OBJDIR)/kernel/kclock.o $(OBJDIR)/kernel/pmap.o

LIB_OBJS := $(OBJDIR)/lib/printf.o $(OBJDIR)/lib/vsprintf.o $(OBJDIR)/lib/string.o
LIB_OBJS += $(OBJDIR)/lib/readline.o

all: $(OBJDIR)/jos.img

# KERNEL_OBJS
$(OBJDIR)/kernel/entry.o: $(KERNEL_SRCDIR)/entry.asm
	@echo + as $@
	@mkdir -p $(@D)
	@$(AS) $(ASFLAGS) $< -o $@

$(OBJDIR)/kernel/lib.o: $(KERNEL_SRCDIR)/lib.asm
	@echo + as $@
	@$(AS) $(ASFLAGS) $< -o $@

$(OBJDIR)/kernel/console.o: $(KERNEL_SRCDIR)/console.c
	@echo + cc $@
	@$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/kernel/init.o: $(KERNEL_SRCDIR)/init.c
	@echo + cc $@
	@$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/kernel/entrypgdir.o: $(KERNEL_SRCDIR)/entrypgdir.c
	@echo + cc $@
	@$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/kernel/monitor.o: $(KERNEL_SRCDIR)/monitor.c
	@echo + cc $@
	@$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/kernel/kclock.o: $(KERNEL_SRCDIR)/kclock.c
	@echo + cc $@
	@$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/kernel/pmap.o: $(KERNEL_SRCDIR)/pmap.c
	@echo + cc $@
	@$(CC) $(CFLAGS) $< -o $@

# KERNEL_OBJS
$(OBJDIR)/lib/printf.o: $(LIB_SRCDIR)/printf.c
	@echo + cc $@
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/lib/vsprintf.o: $(LIB_SRCDIR)/vsprintf.c
	@echo + cc $@
	@$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/lib/string.o: $(LIB_SRCDIR)/string.c
	@echo + cc $@
	@$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/lib/readline.o: $(LIB_SRCDIR)/readline.c
	@echo + cc $@
	@$(CC) $(CFLAGS) $< -o $@

# link kernel
$(OBJDIR)/kernel/kernel: $(KERNEL_OBJS) $(LIB_OBJS)
	@echo + ld $@
	@$(LD) $(LDFLAGS) -T $(KERNEL_SRCDIR)/kernel.ld $^ -o $@
	@$(OBJDUMP) -S $@ >$@.objdump

# make jos.img
$(OBJDIR)/jos.img: $(OBJDIR)/kernel/kernel $(OBJDIR)/boot/boot
	@echo + mk $@
	@dd if=/dev/zero of=$@~ count=10000 2>/dev/null
	@dd if=$(OBJDIR)/boot/boot of=$@~ conv=notrunc 2>/dev/null
	@dd if=$(OBJDIR)/kernel/kernel of=$@~ seek=1 conv=notrunc 2>/dev/null
	@mv $@~ $@