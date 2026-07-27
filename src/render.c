// Braille sub-pixel framebuffer + all screen drawing. This file only reads
// simulation state from particle.c/algorithm.c -- it never mutates it.
//
// Color tiers (ascending render-priority order, used by fb_plot's max-check):
//   0  grid / turnstile ring
//   1  far trail, blue      2  far trail, red
//   3  near trail, blue     4  near trail, red
//   5  live particle, blue  6  live particle, red
//   7  white-hot ember core (fresh paradox)
//   8  orange ember
//   9  dim ember afterglow (fading out)
#include "tenet.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static unsigned char dotmask[ROWS][COLS];
static unsigned char colorbuf[ROWS][COLS];

static const int braille_bit[4][2] = {
    {0x01, 0x08}, {0x02, 0x10}, {0x04, 0x20}, {0x40, 0x80}};

void fb_clear(void) {
  memset(dotmask, 0, sizeof(dotmask));
  memset(colorbuf, 0, sizeof(colorbuf));
}

void fb_plot(double fx, double fy, int color) {
  int x = (int)fx, y = (int)fy;
  if (x < 0 || x >= SUBW || y < 0 || y >= SUBH)
    return;
  int cx = x / 2, cy = y / 4;
  int sx = x % 2, sy = y % 4;
  dotmask[cy][cx] |= braille_bit[sy][sx];
  if (color > colorbuf[cy][cx])
    colorbuf[cy][cx] = color;
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
        case 1:
          n += sprintf(out + n, "\x1b[38;5;17m");
          break; // far trail, blue
        case 2:
          n += sprintf(out + n, "\x1b[38;5;52m");
          break; // far trail, red
        case 3:
          n += sprintf(out + n, "\x1b[38;5;25m");
          break; // near trail, blue
        case 4:
          n += sprintf(out + n, "\x1b[38;5;124m");
          break; // near trail, red
        case 5:
          n += sprintf(out + n, "\x1b[38;5;39m");
          break; // live blue
        case 6:
          n += sprintf(out + n, "\x1b[38;5;196m");
          break; // live red
        case 7:
          n += sprintf(out + n, "\x1b[38;5;231m");
          break; // white-hot ember
        case 8:
          n += sprintf(out + n, "\x1b[38;5;208m");
          break; // orange ember
        case 9:
          n += sprintf(out + n, "\x1b[38;5;88m");
          break; // dim ember afterglow
        default:
          n += sprintf(out + n, "\x1b[38;5;238m");
          break; // grid / ring
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
    fb_plot(CX + TURNSTILE_R * cos(a), CY + TURNSTILE_R * sin(a), 0);
  }
}

// Plots a short fading trail behind a live particle by reading straight out
// of its own recorded history buffer -- no separate trail storage needed.
// Forward particles trail toward lower history indices (their past);
// inverted particles trail toward higher indices (the part of their life
// they already rewound through).
static void plot_trail(Particle *p) {
  int base, dir, near_tier, far_tier;
  if (p->state == 1) {
    base = p->hlen - 1;
    dir = -1;
    near_tier = 3;
    far_tier = 1;
  } else if (p->state == 2) {
    base = p->hplay;
    dir = +1;
    near_tier = 4;
    far_tier = 2;
  } else
    return;

  for (int k = 1; k <= TRAIL_FAR; k++) {
    int idx = base + dir * k;
    if (idx < 0 || idx >= p->hlen)
      break;
    int tier = (k <= TRAIL_NEAR) ? near_tier : far_tier;
    fb_plot(p->hx[idx], p->hy[idx], tier);
  }
}

void draw(int forward_count, int inverted_count, int returned_count,
          const char *msg) {
  fb_clear();
  draw_turnstile();
  for (int i = 0; i < MAX_PART; i++) {
    Particle *p = &P[i];
    if (p->state == 1) {
      plot_trail(p);
      fb_plot(p->x, p->y, 5);
    } else if (p->state == 2) {
      plot_trail(p);
      fb_plot(p->x, p->y, 6);
    } else if (p->state == 3) {
      fb_plot(p->x, p->y, 3);
    } // frozen: dim, stationary
  }
  for (int y = 0; y < ROWS; y++)
    for (int x = 0; x < COLS; x++)
      if (flash_frames[y][x] > 0) {
        int f = flash_frames[y][x];
        if (f > SCORCH_LIFETIME - 6)
          colorbuf[y][x] = 7; // white-hot core
        else if (f > SCORCH_LIFETIME / 2)
          colorbuf[y][x] = 8; // orange ember
        else
          colorbuf[y][x] = 9; // dim afterglow
        dotmask[y][x] = 0xFF;
        flash_frames[y][x]--;
      }
  fb_present();

  static const char *phase_name[] = {"idle", "squad A inbound", "pincer live",
                                     "complete"};
  printf("\x1b[0m\x1b[%d;1H", ROWS + 1);
  printf("\x1b[38;5;39mF:%-3d \x1b[38;5;196mI:%-3d \x1b[38;5;250mR:%-3d "
         "\x1b[38;5;244mTS:%-3d \x1b[38;5;231mPX:%-3d "
         "\x1b[38;5;214m[%s]\x1b[0m %s [space]wave [p]pincer [a]algorithm "
         "[i]force-inv [r]reset [q]quit\x1b[K",
         forward_count, inverted_count, returned_count, turnstile_crossings,
         paradox_events, phase_name[algo_phase], msg);

  printf("\x1b[%d;1H\x1b[38;5;245m", ROWS + 2);
  for (int i = 0; i < log_count && i < LOG_LEN; i++)
    printf("%s%s", event_log[i],
           (i < log_count - 1 && i < LOG_LEN - 1) ? "  |  " : "");
  printf("\x1b[0m\x1b[K");
  fflush(stdout);
}