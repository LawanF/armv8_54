CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic

.SUFFIXES: .c .o

.PHONY: all clean

all: assemble emulate

assemble: assemble.o

EMULATE_DEPS := emulate_files
EMULATE_SRC := $(wildcard $(EMULATE_DEPS)/*?.c)
EMULATE_OBJ := $(subst %.c, %.o, $(EMULATE_SRC))
emulate: emulate.o $(EMULATE_OBJ)

clean:
	$(RM) *.o assemble emulate
	
