CC      = gcc
CFLAGS  = -O2 -Wall -Iinclude
SRCS    = src/term.c src/particle.c src/algorithm.c src/render.c src/main.c
BIN     = tenet

$(BIN): $(SRCS) include/tenet.h
	$(CC) $(CFLAGS) -o $(BIN) $(SRCS) -lm

clean:
	rm -f $(BIN) $(BIN).exe *.o