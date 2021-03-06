BOOT_SRCDIR = boot

BOOT_OBJS = $(OBJDIR)/boot/boot.o $(OBJDIR)/boot/main.o $(OBJDIR)/boot/lib.o

all: $(OBJDIR)/boot/boot.img

$(OBJDIR)/boot/boot.o: $(BOOT_SRCDIR)/boot.asm
	mkdir -p $(@D)
	$(AS) $(ASFLAGS) -I$(BOOT_SRCDIR)/ $^ -o $@

$(OBJDIR)/boot/main.o: $(BOOT_SRCDIR)/main.c
	$(CC) $(CFLAGS) $^ -c -o $@

$(OBJDIR)/boot/lib.o: $(BOOT_SRCDIR)/lib.asm
	$(AS) $(ASFLAGS) $^ -o $@

# ld options:
# -N  Set the text and data sections to be readable and writable.
# -e  entry
#
# objcopy options:
# -S         Do not copy relocation and symbol information from the source file.
# -O binary  generate a raw binary file
# -j         section pattern
$(OBJDIR)/boot/boot: $(BOOT_OBJS)
	$(LD) $(LDFLAGS) $^ -o $@.out
	$(OBJDUMP) -S $@.out >$@.objdump
	$(OBJCOPY) -S -O binary -j .text $@.out $@
	$(PERL) $(BOOT_SRCDIR)/sign.pl $@

$(OBJDIR)/boot/boot.img: $(OBJDIR)/boot/boot
	dd if=/dev/zero of=$@~ count=10000 2>/dev/null
	dd if=$< of=$@~ conv=notrunc 2>/dev/null
	mv $@~ $@
