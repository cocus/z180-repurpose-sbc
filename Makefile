
OBJS = crt0.rel main.rel
H = z180_internal.h
NAME = z180
CFLAGS = -mz180 --code-loc 0x0000 --data-loc 0x8000 --no-std-crt0
ASFLAGS = -plosgff

$(NAME).ihx: $(OBJS)
	@echo "LINK $^ -> $@"
	@sdcc $(CFLAGS) $^ -o $@

%.rel : %.c $(H)
	@echo "SDCC $< -> $@"
	@sdcc $(CFLAGS) -c $< -o $@

%.rel: %.s
	@echo "SDAS $< -> $@"
	@sdasz80 $(ASFLAGS) $(notdir $<)

clean:
	@rm -f $(OBJS) $(OBJS:.rel=.asm) $(OBJS:.rel=.lst) $(OBJS:.rel=.sym) $(NAME).ihx $(NAME).lk $(NAME).map $(NAME).noi
