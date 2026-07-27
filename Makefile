CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -std=c11 -Iinclude
SRCS    = src/term.c src/particle.c src/algorithm.c src/render.c src/main.c
OBJS    = $(SRCS:.c=.o)

# Windows gets .exe; Linux/macOS gets a bare binary
ifeq ($(OS),Windows_NT)
  BIN = tenet.exe
else
  BIN = tenet
endif

$(BIN): $(SRCS) include/tenet.h
	$(CC) $(CFLAGS) -o $(BIN) $(SRCS) -lm

clean:
	rm -f $(BIN) tenet tenet.exe src/*.o