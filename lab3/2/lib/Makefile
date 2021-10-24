LIB_SRCS := lib/readline.c \
	lib/string.c \
	lib/vsprintf.c \
	lib/entry.asm \
	lib/libmain.c

LIB_OBJS := $(patsubst lib/%.c, $(OBJDIR)/lib/%.o, $(LIB_SRCS))
LIB_OBJS := $(patsubst lib/%.asm, $(OBJDIR)/lib/%.o, $(LIB_OBJS))

$(OBJDIR)/lib/%.o: lib/%.c
	@echo + cc $<
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/lib/%.o: lib/%.asm
	@echo + as $<
	@mkdir -p $(@D)
	@$(AS) $(ASFLAGS) $< -o $@

$(OBJDIR)/lib/libjos.a: $(LIB_OBJS)
	@echo + ar $@
	@$(AR) r $@ $(LIB_OBJS)
