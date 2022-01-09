BOOT_SRCS := boot/boot.asm boot/main.c

BOOT_OBJS := $(patsubst boot/%.c, $(OBJDIR)/boot/%.o, $(BOOT_SRCS))
BOOT_OBJS := $(patsubst boot/%.asm, $(OBJDIR)/boot/%.o, $(BOOT_OBJS))

$(OBJDIR)/boot/%.o: boot/%.asm
	@echo + as $<
	@mkdir -p $(@D)
	@$(AS) $(ASFLAGS) -Iboot/ $^ -o $@

# gcc options:
# -Os  Optimize for size.  -Os enables all -O2 optimizations that do not typically
#      increase code size. It also performs further optimizations designed to reduce code size.
$(OBJDIR)/boot/%.o: boot/%.c
	@echo + cc $<
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $^ -Os -o $@

# ld options:
# -N  Set the text and data sections to be readable and writable.
# -e  entry
#
# objcopy options:
# -S         Do not copy relocation and symbol information from the source file.
# -O binary  generate a raw binary file
# -j         section pattern
$(OBJDIR)/boot/boot: $(BOOT_OBJS)
	@echo + mk $@
	@$(LD) $(LDFLAGS) -N -e start -Ttext 0x7C00 $^ -o $@.out
	@$(OBJDUMP) -S $@.out >$@.objdump
	@$(OBJCOPY) -S -O binary -j .text $@.out $@
	@$(PERL) boot/sign.pl $@
