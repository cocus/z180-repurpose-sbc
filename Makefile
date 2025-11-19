
OBJS = crt0.rel main.rel freertos/list.rel freertos/croutine.rel freertos/event_groups.rel freertos/queue.rel freertos/tasks.rel freertos/timers.rel freertos/portable/MemMang/heap_4.rel port.rel
H = z180_internal.h portmacro.h
NAME = z180
CFLAGS = -mz180 --code-loc 0x0000 --data-loc 0x8000 --no-std-crt0 -Ifreertos/ -Ifreertos/include -D__CPU_CLOCK=18432000 -I.
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
