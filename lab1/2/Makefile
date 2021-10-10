OBJDIR = obj
TOP = .

CC = gcc
AS = nasm
LD = ld
OBJCOPY = objcopy
OBJDUMP = objdump
PERL    = perl
QEMU    = qemu-system-i386

CFLAGS  = -fno-builtin -fno-stack-protector -nostdinc -I$(TOP)
CFLAGS += -Wall -Wno-format -Wno-unused -Werror -gstabs -m32
ASFLAGS = -f elf
LDFLAGS = -m elf_i386

GDBPORT = 1234
QEMUOPTS = -drive file=$(OBJDIR)/jos.img,index=0,media=disk,format=raw -serial mon:stdio -gdb tcp::$(GDBPORT)

all:

include boot/Makefile.boot
include kernel/Makefile.kernel

clean:
	rm -r $(OBJDIR)

gdb:
	gdb -n -x .gdbinit

qemu:
	$(QEMU) $(QEMUOPTS)

# qemu options:
# -S  freeze CPU at startup (use 'c' to start execution)
qemu-gdb:
	$(QEMU) $(QEMUOPTS) -S
