CC      = gcc
CFLAGS  = -O2 -Wall -Iinclude
SRCS    = src/term.c src/particle.c src/algorithm.c src/render.c src/main.c
BIN     = tenet
IMGBIN  = img2art

all: $(BIN) $(IMGBIN)

$(BIN): $(SRCS) include/tenet.h
	$(CC) $(CFLAGS) -o $(BIN) $(SRCS) -lm

$(IMGBIN): src/img2art.c include/stb_image.h
	$(CC) $(CFLAGS) -o $(IMGBIN) src/img2art.c -lm

clean:
	rm -f $(BIN) $(BIN).exe $(IMGBIN) $(IMGBIN).exe *.o