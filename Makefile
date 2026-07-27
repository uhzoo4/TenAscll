CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99 -Iinclude
SRC = src/main.c src/term.c src/particle.c src/algorithm.c src/render.c
OBJ = $(SRC:.c=.o)
TARGET = tenet.exe

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
# Cross-platform clean rule
ifeq ($(OS),Windows_NT)
	-cmd /c del /f /q src\*.o *.exe 2>NUL
else
	-rm -f src/*.o *.exe
endif

.PHONY: all clean
