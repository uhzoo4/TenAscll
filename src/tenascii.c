#include "../include/tenascii.h"

AsciiCanvas* create_canvas(int width, int height) {
    AsciiCanvas *canvas = (AsciiCanvas*)malloc(sizeof(AsciiCanvas));
    if (!canvas) return NULL;

    canvas->width = width;
    canvas->height = height;
    canvas->data = (char*)malloc((size_t)(width * height));
    if (!canvas->data) {
        free(canvas);
        return NULL;
    }

    clear_canvas(canvas, ' ');
    return canvas;
}

void free_canvas(AsciiCanvas *canvas) {
    if (canvas) {
        if (canvas->data) free(canvas->data);
        free(canvas);
    }
}

void clear_canvas(AsciiCanvas *canvas, char fill_char) {
    if (!canvas || !canvas->data) return;
    memset(canvas->data, fill_char, (size_t)(canvas->width * canvas->height));
}

void draw_box(AsciiCanvas *canvas, int x, int y, int w, int h) {
    if (!canvas || !canvas->data) return;

    for (int r = y; r < y + h && r < canvas->height; r++) {
        for (int c = x; c < x + w && c < canvas->width; c++) {
            if (r < 0 || c < 0) continue;

            if (r == y || r == y + h - 1) {
                canvas->data[r * canvas->width + c] = '-';
            } else if (c == x || c == x + w - 1) {
                canvas->data[r * canvas->width + c] = '|';
            }
        }
    }
    // Corners
    if (y >= 0 && y < canvas->height && x >= 0 && x < canvas->width) canvas->data[y * canvas->width + x] = '+';
    if (y >= 0 && y < canvas->height && x + w - 1 >= 0 && x + w - 1 < canvas->width) canvas->data[y * canvas->width + (x + w - 1)] = '+';
    if (y + h - 1 >= 0 && y + h - 1 < canvas->height && x >= 0 && x < canvas->width) canvas->data[(y + h - 1) * canvas->width + x] = '+';
    if (y + h - 1 >= 0 && y + h - 1 < canvas->height && x + w - 1 >= 0 && x + w - 1 < canvas->width) canvas->data[(y + h - 1) * canvas->width + (x + w - 1)] = '+';
}

void draw_text(AsciiCanvas *canvas, int x, int y, const char *text) {
    if (!canvas || !canvas->data || !text) return;
    if (y < 0 || y >= canvas->height) return;

    int len = (int)strlen(text);
    for (int i = 0; i < len; i++) {
        int cx = x + i;
        if (cx >= 0 && cx < canvas->width) {
            canvas->data[y * canvas->width + cx] = text[i];
        }
    }
}

void render_canvas(const AsciiCanvas *canvas) {
    if (!canvas || !canvas->data) return;

    for (int r = 0; r < canvas->height; r++) {
        for (int c = 0; c < canvas->width; c++) {
            putchar(canvas->data[r * canvas->width + c]);
        }
        putchar('\n');
    }
}

void print_banner(void) {
    printf(COLOR_CYAN COLOR_BOLD);
    printf("=====================================================\n");
    printf("  _____ _____ _   _    _    ____   ____ ___ ___ \n");
    printf(" |_   _| ____| \\ | |  / \\  / ___| / ___|_ _|_ _|\n");
    printf("   | | |  _| |  \\| | / _ \\ \\___ \\| |    | | | | \n");
    printf("   | | | |___| |\\  |/ ___ \\ ___) | |___ | | | | \n");
    printf("   |_| |_____|_| \\_/_/   \\_\\____/ \\____|___|___|\n");
    printf("                                                    \n");
    printf("       TENASCII C Engine v1.0.0 Ready!              \n");
    printf("=====================================================\n");
    printf(COLOR_RESET "\n");
}
