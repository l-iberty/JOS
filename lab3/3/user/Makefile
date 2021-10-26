$(OBJDIR)/user/%.o: user/%.c
	@echo + cc[USER] $<
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/user/%: $(OBJDIR)/user/%.o $(OBJDIR)/lib/entry.o $(OBJDIR)/lib/libmain.o $(OBJDIR)/lib/libjos.a
	@echo + ld $@
	@$(LD) -o $@ $(LDFLAGS) $^ -T user/user.ld
	@$(OBJDUMP) -S $@ > $@.objdump
	@$(NM) -n $@ > $@.sym