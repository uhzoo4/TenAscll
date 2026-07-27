#ifndef TENET_H
#define TENET_H

// ---- world / framebuffer dimensions ----
#define COLS 110
#define ROWS 42
#define SUBW (COLS * 2)
#define SUBH (ROWS * 4)

#define MAX_PART 400
#define SCORCH_LIFETIME 42 // frames an impact site smoulders for (~1.4s @30fps)
#define TRAIL_NEAR 3       // history points shown at "near" trail brightness
#define TRAIL_FAR                                                              \
  7 // history points shown at "near"+"far" trail brightness combined
#define LOG_LEN 3 // event-log entries kept on screen

typedef struct {
  double x, y, vx, vy;
  double *hx, *hy; // recorded history (grows via realloc doubling)
  int hlen, hcap;
  int hplay; // playback cursor when inverted
  int state; // 0 dead, 1 forward, 2 inverted, 3 returned (frozen)
} Particle;

// ---- shared simulation state ----
// defined in particle.c unless noted
extern Particle P[MAX_PART];
extern int flash_frames[ROWS][COLS];
extern double CX, CY; // singularity center, set once in main.c
extern double G;
extern double TURNSTILE_R;
extern int turnstile_crossings;
extern int paradox_events;
extern int cumulative_returns;
extern char event_log[LOG_LEN][80];
extern int log_count;

// defined in algorithm.c
extern int algo_phase; // 0 idle, 1 squad A inbound, 2 pincer live, 3 complete
extern int algo_baseline;
extern int algo_timer;
extern const int ALGO_TRIGGER;
extern const int ALGO_RUN_FRAMES;

// ---- term.c ----
void term_raw(void);

// ---- particle.c ----
void log_event(const char *fmt, ...);
void hist_push(Particle *p, double x, double y);
void spawn_wave(int from_right);
void invert_all_forward(void);
void reset_sim(void);
void physics_step(double dt);

// ---- algorithm.c ----
void algo_start(void);
const char *algo_tick(void);

// ---- render.c ----
void fb_clear(void);
void fb_plot(double fx, double fy, int color);
void fb_present(void);
void draw_turnstile(void);
void draw(int forward_count, int inverted_count, int returned_count,
          const char *msg);

#endif