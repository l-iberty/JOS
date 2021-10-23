KERNEL_SRCS := kernel/console.c \
	kernel/entry.asm \
	kernel/entrypgdir.c \
	kernel/env.c \
	kernel/init.c \
	kernel/kclock.c \
	kernel/lib.asm \
	kernel/monitor.c \
	kernel/pmap.c \
	kernel/trap.c

KERNEL_BINS := user/hello

KERNEL_OBJS := $(patsubst kernel/%.c, $(OBJDIR)/kernel/%.o, $(KERNEL_SRCS))
KERNEL_OBJS := $(patsubst kernel/%.asm, $(OBJDIR)/kernel/%.o, $(KERNEL_OBJS))

KERNEL_BINS := $(patsubst %, $(OBJDIR)/%, $(KERNEL_BINS))
$(info $(KERNEL_BINS))

$(OBJDIR)/kernel/%.o: kernel/%.c
	@echo + cc $@
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/kernel/%.o: kernel/%.asm
	@echo + cc $@
	@mkdir -p $(@D)
	@$(AS) $(ASFLAGS) $< -o $@

all: $(OBJDIR)/jos.img

# link kernel
$(OBJDIR)/kernel/kernel: $(KERNEL_OBJS) $(LIB_OBJS)
	@echo + ld $@
	@$(LD) $(LDFLAGS) -T kernel/kernel.ld $^ -o $@
	@$(OBJDUMP) -S $@ >$@.objdump

# make jos.img
$(OBJDIR)/jos.img: $(OBJDIR)/kernel/kernel $(OBJDIR)/boot/boot
	@echo + mk $@
	@dd if=/dev/zero of=$@~ count=10000 2>/dev/null
	@dd if=$(OBJDIR)/boot/boot of=$@~ conv=notrunc 2>/dev/null
	@dd if=$(OBJDIR)/kernel/kernel of=$@~ seek=1 conv=notrunc 2>/dev/null
	@mv $@~ $@