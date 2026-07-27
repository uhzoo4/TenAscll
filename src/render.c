#include "../include/tenet.h"

static unsigned char dotmask[ROWS][COLS];
static unsigned char colorbuf[ROWS][COLS];

static const int braille_bit[4][2] = {
    {0x01, 0x08}, {0x02, 0x10}, {0x04, 0x20}, {0x40, 0x80}
};

void fb_clear(void) {
    memset(dotmask, 0, sizeof(dotmask));
    memset(colorbuf, 0, sizeof(colorbuf));
}

void fb_plot(double fx, double fy, int color) {
    int x = (int)fx, y = (int)fy;
    if (x < 0 || x >= SUBW || y < 0 || y >= SUBH) return;
    int cx = x / 2, cy = y / 4;
    int sx = x % 2, sy = y % 4;
    dotmask[cy][cx] |= braille_bit[sy][sx];
    if (color > colorbuf[cy][cx]) colorbuf[cy][cx] = color;
}

void fb_present(void) {
    static char out[(ROWS) * (COLS * 24 + 16)];
    int n = 0;
    n += sprintf(out + n, "\x1b[H");
    int last_color = -1;
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            unsigned char m = dotmask[y][x];
            unsigned char c = colorbuf[y][x];
            if (c != last_color) {
                switch (c) {
                case 1: n += sprintf(out + n, "\x1b[38;5;39m"); break;  // Blue (forward)
                case 2: n += sprintf(out + n, "\x1b[38;5;196m"); break; // Red (inverted)
                case 3: n += sprintf(out + n, "\x1b[38;5;231m"); break; // White-hot core
                case 4: n += sprintf(out + n, "\x1b[38;5;208m"); break; // Orange ember
                case 5: n += sprintf(out + n, "\x1b[38;5;88m"); break;  // Dim ember afterglow
                default: n += sprintf(out + n, "\x1b[38;5;238m"); break;// Dim grey grid
                }
                last_color = c;
            }
            unsigned int cp = 0x2800 + m;
            out[n++] = (char)(0xE0 | (cp >> 12));
            out[n++] = (char)(0x80 | ((cp >> 6) & 0x3F));
            out[n++] = (char)(0x80 | (cp & 0x3F));
        }
        out[n++] = '\r';
        out[n++] = '\n';
    }
    fwrite(out, 1, n, stdout);
    fflush(stdout);
}

void draw_turnstile(void) {
    int steps = 90;
    for (int s = 0; s < steps; s++) {
        double a = 2.0 * M_PI * s / steps;
        fb_plot(CX_POS + TURNSTILE_R * cos(a), CY_POS + TURNSTILE_R * sin(a), 0);
    }
}

void draw(int forward_count, int inverted_count, int returned_count, const char *msg) {
    fb_clear();
    draw_turnstile();
    for (int i = 0; i < MAX_PART; i++) {
        Particle *p = &P[i];
        if (p->state == 1) fb_plot(p->x, p->y, 1);
        else if (p->state == 2) fb_plot(p->x, p->y, 2);
        else if (p->state == 3) fb_plot(p->x, p->y, 1);
    }
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            if (flash_frames[y][x] > 0) {
                int f = flash_frames[y][x];
                if (f > SCORCH_LIFETIME - 6) colorbuf[y][x] = 3;
                else if (f > SCORCH_LIFETIME / 2) colorbuf[y][x] = 4;
                else colorbuf[y][x] = 5;
                dotmask[y][x] = 0xFF;
                flash_frames[y][x]--;
            }
        }
    }
    fb_present();
    static const char *phase_name[] = {"idle", "squad A inbound", "pincer live", "complete"};
    printf("\x1b[0m\x1b[%d;1H", ROWS + 1);
    printf("\x1b[38;5;39mF:%-3d \x1b[38;5;196mI:%-3d \x1b[38;5;250mR:%-3d "
           "\x1b[38;5;244mTS:%-3d \x1b[38;5;231mPX:%-3d "
           "\x1b[38;5;214m[%s]\x1b[0m %s [space]wave [p]pincer [a]algorithm "
           "[i]force-inv [r]reset [q]quit\x1b[K",
           forward_count, inverted_count, returned_count, turnstile_crossings,
           paradox_events, phase_name[algo_phase], msg);
    fflush(stdout);
}
