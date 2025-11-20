
OBJS = crt0.rel main.rel freertos/list.rel freertos/croutine.rel freertos/event_groups.rel freertos/queue.rel freertos/tasks.rel freertos/timers.rel freertos/portable/MemMang/heap_4.rel port.rel
H = z180_internal.h portmacro.h
NAME = z180
CFLAGS = -mz180 --code-loc 0x0000 --data-loc 0x8000 --no-std-crt0 -Ifreertos/ -Ifreertos/include -D__CPU_CLOCK=18432000 -I.
ASFLAGS = -plosgff

all: size

size: $(NAME).ihx $(NAME).map
# horrible, but works :)
	@echo "*          Code at 0x$$(grep '_CODE' $(NAME).map | sed -n '3 p' | cut -d 'E' -f2 | sed 's/^[ \t]*//;s/[ \t]*$$//' | cut -d ' ' -f1), 0x$$(grep '_GSFINAL' $(NAME).map | sed -n '3 p' | cut -d 'L' -f2 | sed 's/^[ \t]*//;s/[ \t]*$$//' | cut -d ' ' -f1) bytes"
	@echo "*      RAM Data at 0x$$(grep '_DATA' $(NAME).map | sed -n '3 p' | cut -d 'A' -f3 | sed 's/^[ \t]*//;s/[ \t]*$$//' | cut -d ' ' -f1), 0x$$(grep '_DATA' $(NAME).map | sed -n '3 p' | cut -d 'A' -f3 | sed 's/^[ \t]*//;s/[ \t]*$$//' | cut -d ' ' -f5) bytes"
	@echo "* RAM Init Data at 0x$$(grep '_INITIALIZED' $(NAME).map | sed -n '3 p' | cut -d 'D' -f2 | sed 's/^[ \t]*//;s/[ \t]*$$//' | cut -d ' ' -f1), 0x$$(grep '_INITIALIZED' $(NAME).map | sed -n '3 p' | cut -d 'D' -f2 | sed 's/^[ \t]*//;s/[ \t]*$$//' | cut -d ' ' -f5) bytes"

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
