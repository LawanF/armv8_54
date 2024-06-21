CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -pedantic
LDFLAGS ?= -lm -lSDL2 -lncurses

.SUFFIXES: .c .o

.PHONY: all clean

all: synthesize visualize

synthesize: synthesizer.o engine/envelope.o input/keyboard.o input/settings.o engine/waveforms.o
	cc $(LDFLAGS) $(CFLAGS) $^ -o synthesize
synthesizer.o: synthesizer.c
engine/envelope.o: engine/envelope.c
engine/waveforms.o: engine/waveforms.c
input/keyboard.o: input/keyboard.c
input/settings.o: input/settings.c

visualize: visualizer.o $(VISUALIZER_OBJ) $(ENGINE_OBJ) $(INPUT_OBJ)

clean:
	$(RM) **/*.o *.o synthesize visualize a.out
	
