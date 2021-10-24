$(OBJDIR)/user/%.o: user/%.c
	@echo + cc[USER] $<
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/user/%: $(OBJDIR)/user/%.o $(OBJDIR)/lib/entry.o $(OBJDIR)/lib/libjos.a
	@echo + ld $@
	@$(LD) $(LDFLAGS) -T user/user.ld $^ -o $@
	@$(OBJDUMP) -S $@ > $@.objdump
	@$(NM) -n $@ > $@.sym
