OBJDIR = obj
TOP = .

CC = gcc
AS = nasm
LD = ld
OBJCOPY = objcopy
OBJDUMP = objdump
PERL    = perl
QEMU    = qemu-system-i386

CFLAGS  = -fno-builtin -fno-stack-protector -nostdinc -m32 -gstabs
ASFLAGS = -f elf
LDFLAGS = -m elf_i386 -N -e start -Ttext 0x7C00

GDBPORT = 1234
QEMUOPTS = -drive file=$(OBJDIR)/boot/boot.img,index=0,media=disk,format=raw -serial mon:stdio -gdb tcp::$(GDBPORT)

all:

include boot/Makefile.boot

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
