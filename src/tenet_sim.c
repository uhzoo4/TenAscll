// TENET — time-inversion particle playground
// Blue = forward entropy, Red = inverted entropy (retracing its own past).
// A forward/inverted collision is a real paradox now: both particles are
// annihilated on contact, and the site burns down through a white-hot core
// into a fading ember instead of a single flash frame.
//
// Controls:
//   SPACE  launch a forward wave from the left
//   p      launch an unscripted pincer wave from the right (forward)
//   a      run THE ALGORITHM — the scripted, timed pincer: launches a
//          forward squad, waits until enough of it has inverted through
//          the turnstile, then launches the second squad so it meets the
//          first squad's retreating (inverted) particles head-on
//   i      manual override — force-invert every currently-forward particle
//          immediately (debug key; the real mechanic is the turnstile below)
//   r      reset everything
//   q      quit
//
// The turnstile: a ring drawn around the central singularity. Any forward
// (blue) particle that crosses INTO that ring is automatically inverted —
// it flips to red and starts rewinding its own recorded history, walking
// back out the way it came. This is the actual film mechanic: crossing the
// turnstile is what flips your entropy, not a button press.
//
// The Algorithm ties this into deliberate choreography: it delays the
// second squad's launch until the turnstile has actually converted a
// chunk of the first squad, so the two squads are guaranteed to occupy
// the same space while moving in opposite entropy directions — a real
// pincer movement instead of two waves that happen to overlap by luck.
//
// Paradox consequence: forward matter meeting its own inverted matter is
// not survivable. Both particles are deleted from the simulation at the
// point of contact — the dot count you see visibly thins out as the pincer
// converges — and the impact site smoulders for ~1.4s (white core, then
// orange, then a dim ember) so the event reads as consequential rather
// than cosmetic.
//
// Build:  gcc -O2 -o tenet tenet_sim.c -lm
// Run:    ./tenet   (use a real terminal, at least 100x40, UTF-8 + truecolor)

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef _WIN32
#include <windows.h>
#include <conio.h>

static HANDLE hStdin, hStdout;
static DWORD orig_in_mode, orig_out_mode;

static void term_restore(void) {
  SetConsoleMode(hStdin, orig_in_mode);
  SetConsoleMode(hStdout, orig_out_mode);
  printf("\x1b[?25h\x1b[?1049l"); // show cursor, leave alt screen
  fflush(stdout);
}

static void term_raw(void) {
  hStdin = GetStdHandle(STD_INPUT_HANDLE);
  hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
  GetConsoleMode(hStdin, &orig_in_mode);
  GetConsoleMode(hStdout, &orig_out_mode);

  DWORD raw_in = orig_in_mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
  SetConsoleMode(hStdin, raw_in);

  DWORD raw_out = orig_out_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  SetConsoleMode(hStdout, raw_out);

  printf("\x1b[?1049h\x1b[?25l"); // alt screen, hide cursor
  atexit(term_restore);
}

static int get_char_nonblock(char *c) {
  if (_kbhit()) {
    *c = (char)_getch();
    return 1;
  }
  return 0;
}

static void sleep_ms(int ms) {
  Sleep(ms);
}

#else
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

static struct termios orig_term;
static void term_restore(void) {
  tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);
  printf("\x1b[?25h\x1b[?1049l"); // show cursor, leave alt screen
  fflush(stdout);
}
static void term_raw(void) {
  tcgetattr(STDIN_FILENO, &orig_term);
  struct termios raw = orig_term;
  raw.c_lflag &= ~(ECHO | ICANON);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &raw);
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
  printf("\x1b[?1049h\x1b[?25l"); // alt screen, hide cursor
  atexit(term_restore);
}

static int get_char_nonblock(char *c) {
  return read(STDIN_FILENO, c, 1) == 1;
}

static void sleep_ms(int ms) {
  struct timespec ts = {0, ms * 1000000L};
  nanosleep(&ts, NULL);
}
#endif

// ---------- braille framebuffer ----------
// Each terminal cell = 2x4 subpixel block via Unicode braille patterns.
#define COLS 110
#define ROWS 42
#define SUBW (COLS * 2)
#define SUBH (ROWS * 4)

static unsigned char dotmask[ROWS][COLS]; // which subpixels are lit
static unsigned char
    colorbuf[ROWS][COLS]; // 0 none, 1 blue, 2 red, 3 white(collision)

static const int braille_bit[4][2] = {
    {0x01, 0x08}, {0x02, 0x10}, {0x04, 0x20}, {0x40, 0x80}};

static void fb_clear(void) {
  memset(dotmask, 0, sizeof(dotmask));
  memset(colorbuf, 0, sizeof(colorbuf));
}

static void fb_plot(double fx, double fy, int color) {
  int x = (int)fx, y = (int)fy;
  if (x < 0 || x >= SUBW || y < 0 || y >= SUBH)
    return;
  int cx = x / 2, cy = y / 4;
  int sx = x % 2, sy = y % 4;
  dotmask[cy][cx] |= braille_bit[sy][sx];
  // priority: collision white > inverted red > forward blue
  if (color > colorbuf[cy][cx])
    colorbuf[cy][cx] = color;
}

static void fb_present(void) {
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
          n += sprintf(out + n, "\x1b[38;5;39m");
          break; // blue
        case 2:
          n += sprintf(out + n, "\x1b[38;5;196m");
          break; // red
        case 3:
          n += sprintf(out + n, "\x1b[38;5;231m");
          break; // white-hot core
        case 4:
          n += sprintf(out + n, "\x1b[38;5;208m");
          break; // orange ember
        case 5:
          n += sprintf(out + n, "\x1b[38;5;88m");
          break; // dim ember afterglow
        default:
          n += sprintf(out + n, "\x1b[38;5;238m");
          break; // dim grey grid
        }
        last_color = c;
      }
      unsigned int cp = 0x2800 + m;
      // encode codepoint as UTF-8 (always in 3-byte range here)
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

// ---------- particles ----------
#define MAX_PART 400
#define HIST_STEP 1 // record every frame

typedef struct {
  double x, y, vx, vy;
  double *hx, *hy; // recorded history
  int hlen, hcap;
  int hplay; // playback index when inverted
  int state; // 0 dead, 1 forward, 2 inverted, 3 returned(frozen)
} Particle;

static Particle P[MAX_PART];
static int flash_frames[ROWS][COLS]; // collision flash timers, in char-cells

static double CX_POS, CY_POS; // singularity center (subpixel coords)
static double G = 1400.0;
static double TURNSTILE_R =
    34.0; // ring radius (sub-pixels) that triggers inversion
static int turnstile_crossings = 0; // lifetime count of auto-inversions
static int paradox_events = 0; // lifetime count of forward/inverted collisions
#define SCORCH_LIFETIME 42 // frames an impact site smoulders for (~1.4s @30fps)

// --- The Algorithm: scripted two-squad pincer choreography ---
// 0 idle, 1 squad A launched (waiting on turnstile), 2 squad B launched, 3 done
static int algo_phase = 0;
static int algo_baseline = 0; // turnstile_crossings count when squad A launched
static int algo_timer = 0;    // frames spent in phase 2, for a completion cue
static const int ALGO_TRIGGER =
    15; // crossings needed from squad A before squad B launches
static const int ALGO_RUN_FRAMES =
    240; // ~8s at 30fps before declaring the run complete

static void hist_push(Particle *p, double x, double y) {
  if (p->hlen >= p->hcap) {
    p->hcap = p->hcap ? p->hcap * 2 : 64;
    p->hx = realloc(p->hx, sizeof(double) * p->hcap);
    p->hy = realloc(p->hy, sizeof(double) * p->hcap);
  }
  p->hx[p->hlen] = x;
  p->hy[p->hlen] = y;
  p->hlen++;
}

static void spawn_wave(int from_right) {
  for (int i = 0; i < MAX_PART; i++) {
    if (P[i].state != 0)
      continue;
    int n_to_spawn = 40;
    for (int k = 0; k < n_to_spawn && i < MAX_PART; k++, i++) {
      if (P[i].state != 0)
        continue;
      Particle *p = &P[i];
      free(p->hx);
      free(p->hy);
      memset(p, 0, sizeof(*p));
      double t = (double)k / n_to_spawn;
      if (!from_right) {
        p->x = 4;
        p->y = 2 + t * (SUBH - 4);
        p->vx = 60 + 20 * ((double)rand() / RAND_MAX);
        p->vy = 10 * (((double)rand() / RAND_MAX) - 0.5);
      } else {
        p->x = SUBW - 4;
        p->y = 2 + t * (SUBH - 4);
        p->vx = -(60 + 20 * ((double)rand() / RAND_MAX));
        p->vy = 10 * (((double)rand() / RAND_MAX) - 0.5);
      }
      p->state = 1;
      hist_push(p, p->x, p->y);
    }
    break;
  }
}

static void invert_all_forward(void) {
  for (int i = 0; i < MAX_PART; i++) {
    if (P[i].state == 1 && P[i].hlen > 1) {
      P[i].state = 2;
      P[i].hplay = P[i].hlen - 1;
    }
  }
}

static void reset_sim(void) {
  for (int i = 0; i < MAX_PART; i++) {
    free(P[i].hx);
    free(P[i].hy);
    memset(&P[i], 0, sizeof(P[i]));
  }
  memset(flash_frames, 0, sizeof(flash_frames));
  turnstile_crossings = 0;
  paradox_events = 0;
  algo_phase = 0;
  algo_baseline = 0;
  algo_timer = 0;
}

static void physics_step(double dt) {
  for (int i = 0; i < MAX_PART; i++) {
    Particle *p = &P[i];
    if (p->state == 0 || p->state == 3)
      continue;

    if (p->state == 1) {
      // forward: gravity toward singularity + wall bounce
      double dx0 = CX_POS - p->x, dy0 = CY_POS - p->y;
      double dist_before = sqrt(dx0 * dx0 + dy0 * dy0);

      double r2 = dx0 * dx0 + dy0 * dy0 + 400.0;
      double r = sqrt(r2);
      double f = G / r2;
      p->vx += f * dx0 / r * dt;
      p->vy += f * dy0 / r * dt;
      p->x += p->vx * dt;
      p->y += p->vy * dt;
      if (p->x < 1) {
        p->x = 1;
        p->vx = fabs(p->vx) * 0.85;
      }
      if (p->x > SUBW - 2) {
        p->x = SUBW - 2;
        p->vx = -fabs(p->vx) * 0.85;
      }
      if (p->y < 1) {
        p->y = 1;
        p->vy = fabs(p->vy) * 0.85;
      }
      if (p->y > SUBH - 2) {
        p->y = SUBH - 2;
        p->vy = -fabs(p->vy) * 0.85;
      }
      hist_push(p, p->x, p->y);

      // turnstile: crossing INTO the ring auto-inverts entropy
      double dx1 = CX_POS - p->x, dy1 = CY_POS - p->y;
      double dist_after = sqrt(dx1 * dx1 + dy1 * dy1);
      if (dist_before > TURNSTILE_R && dist_after <= TURNSTILE_R &&
          p->hlen > 1) {
        p->state = 2;
        p->hplay = p->hlen - 1;
        turnstile_crossings++;
      }
    } else if (p->state == 2) {
      // inverted: walk backward through its own recorded life
      if (p->hplay <= 0) {
        p->state = 3; // returned to its origin — frozen
        continue;
      }
      p->hplay--;
      p->x = p->hx[p->hplay];
      p->y = p->hy[p->hplay];
    }
  }

  // collision detection: forward vs inverted sharing a cell is a paradox —
  // both particles are annihilated on contact, not just flashed
  for (int i = 0; i < MAX_PART; i++) {
    if (P[i].state != 1 && P[i].state != 2)
      continue;
    for (int j = i + 1; j < MAX_PART; j++) {
      if (P[i].state != 1 && P[i].state != 2)
        break; // i was annihilated earlier this frame
      if (P[j].state != 1 && P[j].state != 2)
        continue;
      if (P[i].state == P[j].state)
        continue; // need one of each direction
      double dx = P[i].x - P[j].x, dy = P[i].y - P[j].y;
      if (dx * dx + dy * dy < 9.0) {
        int cx = (int)(P[i].x / 2), cy = (int)(P[i].y / 4);
        if (cx >= 0 && cx < COLS && cy >= 0 && cy < ROWS) {
          if (flash_frames[cy][cx] == 0)
            paradox_events++;
          flash_frames[cy][cx] = SCORCH_LIFETIME;
        }
        free(P[i].hx);
        free(P[i].hy);
        memset(&P[i], 0, sizeof(P[i]));
        free(P[j].hx);
        free(P[j].hy);
        memset(&P[j], 0, sizeof(P[j]));
      }
    }
  }
}

static void algo_start(void) {
  reset_sim();
  spawn_wave(0); // squad A: forward, from the left
  algo_phase = 1;
  algo_baseline = turnstile_crossings;
  algo_timer = 0;
}

// Advances the scripted choreography by one frame. Returns a status message
// when the phase changes, or NULL if nothing changed this frame.
static const char *algo_tick(void) {
  if (algo_phase == 1) {
    if (turnstile_crossings - algo_baseline >= ALGO_TRIGGER) {
      spawn_wave(1); // squad B: forward, from the right — timed to
                     // meet squad A's now-inverted survivors head-on
      algo_phase = 2;
      return "ALGORITHM: squad B launched — forward meets inverted";
    }
  } else if (algo_phase == 2) {
    algo_timer++;
    if (algo_timer >= ALGO_RUN_FRAMES) {
      algo_phase = 3;
      return "ALGORITHM complete";
    }
  }
  return NULL;
}

static void draw_turnstile(void) {
  // dim ring outline marking the auto-inversion boundary; color 0 so any
  // particle passing over it always draws on top (priority in fb_plot)
  int steps = 90;
  for (int s = 0; s < steps; s++) {
    double a = 2.0 * M_PI * s / steps;
    fb_plot(CX_POS + TURNSTILE_R * cos(a), CY_POS + TURNSTILE_R * sin(a), 0);
  }
}

static void draw(int forward_count, int inverted_count, int returned_count,
                 const char *msg) {
  fb_clear();
  draw_turnstile();
  for (int i = 0; i < MAX_PART; i++) {
    Particle *p = &P[i];
    if (p->state == 1)
      fb_plot(p->x, p->y, 1);
    else if (p->state == 2)
      fb_plot(p->x, p->y, 2);
    else if (p->state == 3)
      fb_plot(p->x, p->y, 1); // frozen still shown faint blue
  }
  for (int y = 0; y < ROWS; y++)
    for (int x = 0; x < COLS; x++)
      if (flash_frames[y][x] > 0) {
        int f = flash_frames[y][x];
        if (f > SCORCH_LIFETIME - 6)
          colorbuf[y][x] = 3; // white-hot core (first ~0.2s)
        else if (f > SCORCH_LIFETIME / 2)
          colorbuf[y][x] = 4; // orange ember
        else
          colorbuf[y][x] = 5; // dim afterglow, fading out
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
  fflush(stdout);
}

int main(void) {
  term_raw();
  CX_POS = SUBW / 2.0;
  CY_POS = SUBH / 2.0;
  reset_sim();
  spawn_wave(0);

  const char *msg = "temporal pincer movement ready";
  int running = 1;
  while (running) {
    char c;
    while (get_char_nonblock(&c)) {
      if (c == 'q')
        running = 0;
      else if (c == ' ') {
        spawn_wave(0);
        msg = "forward wave launched (blue)";
      } else if (c == 'p') {
        spawn_wave(1);
        msg = "pincer wave launched from the right";
      } else if (c == 'a') {
        algo_start();
        msg = "ALGORITHM engaged: squad A inbound, awaiting turnstile";
      } else if (c == 'i') {
        invert_all_forward();
        msg = "manual override: force-inverted all forward particles";
      } else if (c == 'r') {
        reset_sim();
        spawn_wave(0);
        msg = "reset";
      }
    }

    physics_step(0.5);
    {
      const char *am = algo_tick();
      if (am)
        msg = am;
    }

    int fc = 0, ic = 0, rc = 0;
    for (int i = 0; i < MAX_PART; i++) {
      if (P[i].state == 1)
        fc++;
      else if (P[i].state == 2)
        ic++;
      else if (P[i].state == 3)
        rc++;
    }
    draw(fc, ic, rc, msg);

    sleep_ms(33);
  }
  return 0;
}
