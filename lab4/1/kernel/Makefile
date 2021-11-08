KERNEL_SRCS := kernel/console.c \
	kernel/entry.asm \
	kernel/entrypgdir.c \
	kernel/env.c \
	kernel/_env.asm \
	kernel/init.c \
	kernel/kclock.c \
	kernel/printf.c \
	kernel/syscall.c \
	kernel/lib.asm \
	kernel/lapic.c \
	kernel/monitor.c \
	kernel/mpconfig.c \
	kernel/pmap.c \
	kernel/trap.c \
	kernel/trapentry.asm \
	lib/readline.c \
	lib/string.c \
	lib/vsprintf.c

KERNEL_BINS := user/hello \
	user/divzero \
	user/badsegment \
	user/softint \
	user/invlopcode \
	user/faultread \
	user/faultreadkernel \
	user/faultwrite \
	user/faultwritekernel \
	user/breakpoint

KERNEL_OBJS := $(patsubst kernel/%.c, $(OBJDIR)/kernel/%.o, $(KERNEL_SRCS))
KERNEL_OBJS := $(patsubst kernel/%.asm, $(OBJDIR)/kernel/%.o, $(KERNEL_OBJS))
KERNEL_OBJS := $(patsubst lib/%.c, $(OBJDIR)/lib/%.o, $(KERNEL_OBJS))

KERNEL_BINS := $(patsubst %, $(OBJDIR)/%, $(KERNEL_BINS))

$(OBJDIR)/kernel/%.o: kernel/%.c
	@echo + cc $<
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/kernel/%.o: kernel/%.asm
	@echo + as $<
	@mkdir -p $(@D)
	@$(AS) $(ASFLAGS) $< -o $@

all: $(OBJDIR)/jos.img

# link kernel
$(OBJDIR)/kernel/kernel: $(KERNEL_OBJS) $(KERNEL_BINS)
	@echo + ld $@
	@$(LD) -o $@ $(LDFLAGS) $(KERNEL_OBJS) -T kernel/kernel.ld -b binary $(KERNEL_BINS)
	@$(OBJDUMP) -S $@ >$@.objdump
	@$(NM) -n $@ > $@.sym

# make jos.img
$(OBJDIR)/jos.img: $(OBJDIR)/kernel/kernel $(OBJDIR)/boot/boot
	@echo + mk $@
	@dd if=/dev/zero of=$@~ count=10000 2>/dev/null
	@dd if=$(OBJDIR)/boot/boot of=$@~ conv=notrunc 2>/dev/null
	@dd if=$(OBJDIR)/kernel/kernel of=$@~ seek=1 conv=notrunc 2>/dev/null
	@mv $@~ $@