OBJDIR := obj
TOP := .

CC := gcc
AS := nasm
LD := ld
AR := ar
NM := nm
OBJCOPY := objcopy
OBJDUMP := objdump
PERL    := perl
QEMU    := qemu-system-i386

CFLAGS  := -fno-builtin -fno-stack-protector -nostdinc -fno-common -I$(TOP)
CFLAGS  += -Wall -Wno-format -Wno-unused -Werror -g3 -m32 -c
ASFLAGS := -f elf
LDFLAGS := -m elf_i386

CPUS ?= 1

GDBPORT := 1234
QEMUOPTS := -drive file=$(OBJDIR)/jos.img,index=0,media=disk,format=raw -serial mon:stdio -gdb tcp::$(GDBPORT)
QEMUOPTS += -smp $(CPUS)

all:

include boot/Makefile
include kernel/Makefile
include lib/Makefile
include user/Makefile

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
