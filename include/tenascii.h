#ifndef TENASCII_H
#define TENASCII_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ANSI Color Codes for Rich Terminal Display
#define COLOR_RESET   "\x1b[0m"
#define COLOR_BOLD    "\x1b[1m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_RED     "\x1b[31m"

// ASCII Character Ramp (dark to bright)
#define ASCII_RAMP " .:-=+*#%@"

typedef struct {
    int width;
    int height;
    char *data;
} AsciiCanvas;

// Function Declarations
AsciiCanvas* create_canvas(int width, int height);
void free_canvas(AsciiCanvas *canvas);
void clear_canvas(AsciiCanvas *canvas, char fill_char);
void draw_box(AsciiCanvas *canvas, int x, int y, int w, int h);
void draw_text(AsciiCanvas *canvas, int x, int y, const char *text);
void render_canvas(const AsciiCanvas *canvas);
void print_banner(void);

#endif // TENASCII_H
