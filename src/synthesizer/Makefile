CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -pedantic
LDFLAGS ?= -lm -lSDL2 -lncurses

.SUFFIXES: .c .o

.PHONY: all clean

all: synthesize visualize

INPUT_DEPS := input
INPUT_SRC := $(wildcard $(INPUT_DEPS)/*?.c)
INPUT_OBJ := $(subst %.c, %.o, $(INPUT_SRC))

ENGINE_DEPS := engine
ENGINE_SRC := $(wildcard $(ENGINE_DEPS)/*?.c)
ENGINE_OBJ := $(subst %.c, %.o, $(ENGINE_SRC))

VISUALIZER_DEPS := visualizer
VISUALIZER_SRC := $(wildcard $(VISUALIZER_DEPS)/*?.c)
VISUALIZER_OBJ := $(subst %.c, %.o, $(VISUALIZER_SRC))

synthesize: synthesizer.o engine/envelope.o input/keyboard.o input/settings.o engine/waveforms.o
synthesizer.o: synthesizer.c
engine/envelope.o: engine/envelope.c
engine/waveforms.o: engine/waveforms.c
input/keyboard.o: input/keyboard.c
input/settings.o: input/settings.c

visualize: visualizer.o $(VISUALIZER_OBJ) $(ENGINE_OBJ) $(INPUT_OBJ)

clean:
	$(RM) **/*.o *.o synthesize visualize
	
