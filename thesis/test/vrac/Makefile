SRC:= $(wildcard *.c)
ASLL:=${SRC:.c=.ll}
AS:=${ASLL:.ll=.s}

.SUFFIXES : .ll .c .s

.c.ll	: $@
	@clang $< -emit-llvm -S -o $@ && echo $@" created"

.ll.s	: $@
	@llc -f $< && echo $@ " created"
	@cp $< $<.gen

all	: ${AS}
	@echo "all .s are created"

clean 	:
	@rm -f *~ ${AS} ${ASLL}
	@echo "All are cleaned"
