CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -pedantic\
	-lSDL2 -lm

.SUFFIXES: .c .o

.PHONY: all clean

all: synthesizer

INPUT_DEPS := input
INPUT_SRC := $(wildcard $(INPUT_DEPS)/*?.c)
INPUT_OBJ := $(subst %.c, %.o, $(INPUT_SRC))

ENGINE_DEPS := engine
ENGINE_SRC := $(wildcard $(ENGINE_DEPS)/*?.c)
ENGINE_OBJ := $(subst %.c, %.o, $(ENGINE_SRC))

synthesizer: synthesizer.o INPUT_OBJ ENGINE_OBJ

clean:
	$(RM) *.o synthesizer
	
