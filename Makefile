CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99 -Iinclude
SRC_ENGINE = src/main.c src/tenascii.c
SRC_TENET = src/tenet_sim.c

TARGET_ENGINE = tenascii.exe
TARGET_TENET = tenet.exe

all: $(TARGET_ENGINE) $(TARGET_TENET)

$(TARGET_ENGINE): $(SRC_ENGINE)
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET_TENET): $(SRC_TENET)
	$(CC) $(CFLAGS) -o $@ $^ -lm

clean:
	rm -f *.exe src/*.o

.PHONY: all clean
