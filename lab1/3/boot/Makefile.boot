BOOT_SRCDIR := boot

BOOT_OBJS := $(OBJDIR)/boot/boot.o $(OBJDIR)/boot/main.o

$(OBJDIR)/boot/boot.o: $(BOOT_SRCDIR)/boot.asm
	mkdir -p $(@D)
	$(AS) $(ASFLAGS) -I$(BOOT_SRCDIR)/ $^ -o $@

# gcc options:
# -Os  Optimize for size.  -Os enables all -O2 optimizations that do not typically
#      increase code size. It also performs further optimizations designed to reduce code size.
$(OBJDIR)/boot/main.o: $(BOOT_SRCDIR)/main.c
	$(CC) $(CFLAGS) $^ -Os -o $@

# ld options:
# -N  Set the text and data sections to be readable and writable.
# -e  entry
#
# objcopy options:
# -S         Do not copy relocation and symbol information from the source file.
# -O binary  generate a raw binary file
# -j         section pattern
$(OBJDIR)/boot/boot: $(BOOT_OBJS)
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7C00 $^ -o $@.out
	$(OBJDUMP) -S $@.out >$@.objdump
	$(OBJCOPY) -S -O binary -j .text $@.out $@
	$(PERL) $(BOOT_SRCDIR)/sign.pl $@
