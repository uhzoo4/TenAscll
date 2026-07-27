#ifndef TENET_H
#define TENET_H

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

// ANSI Color Codes for Rich Terminal Display
#define COLOR_RESET   "\x1b[0m"
#define COLOR_BOLD    "\x1b[1m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_RED     "\x1b[31m"

// Terminal & Framebuffer Dimensions
#define COLS 110
#define ROWS 42
#define SUBW (COLS * 2)
#define SUBH (ROWS * 4)

// Particles & Simulation Constants
#define MAX_PART 400
#define HIST_STEP 1
#define SCORCH_LIFETIME 42 // ~1.4s @ 30fps

typedef struct {
    double x, y, vx, vy;
    double *hx, *hy;
    int hlen, hcap;
    int hplay;
    int state; // 0 dead, 1 forward (blue), 2 inverted (red), 3 returned (frozen)
} Particle;

// Shared Global Simulation Variables
extern Particle P[MAX_PART];
extern int flash_frames[ROWS][COLS];
extern double CX_POS, CY_POS;
extern double G;
extern double TURNSTILE_R;
extern int turnstile_crossings;
extern int paradox_events;

extern int algo_phase;
extern int algo_baseline;
extern int algo_timer;
extern const int ALGO_TRIGGER;
extern const int ALGO_RUN_FRAMES;

// Module 1: Terminal Abstraction (term.c)
void term_raw(void);
void term_restore(void);
int get_char_nonblock(char *c);
void sleep_ms(int ms);

// Module 2: Particle Physics & Simulation (particle.c)
void hist_push(Particle *p, double x, double y);
void spawn_wave(int from_right);
void invert_all_forward(void);
void reset_sim(void);
void physics_step(double dt);

// Module 3: Scripted Algorithm Choreography (algorithm.c)
void algo_start(void);
const char *algo_tick(void);

// Module 4: Unicode Braille Render Engine (render.c)
void fb_clear(void);
void fb_plot(double fx, double fy, int color);
void fb_present(void);
void draw_turnstile(void);
void draw(int forward_count, int inverted_count, int returned_count, const char *msg);

#endif // TENET_H
