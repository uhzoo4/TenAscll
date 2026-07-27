// THE ALGORITHM: scripted two-squad pincer choreography. Delays squad B's
// launch until the turnstile has actually converted a chunk of squad A, so
// the two squads are guaranteed to occupy the same space while moving in
// opposite entropy directions -- a real pincer, not a lucky overlap.
#include "tenet.h"
#include <stddef.h>

int algo_phase = 0;
int algo_baseline = 0;
int algo_timer = 0;
const int ALGO_TRIGGER =
    15; // crossings needed from squad A before squad B launches
const int ALGO_RUN_FRAMES =
    240; // ~8s at 30fps before declaring the run complete

void algo_start(void) {
  reset_sim();
  spawn_wave(0); // squad A: forward, from the left
  algo_phase = 1;
  algo_baseline = turnstile_crossings;
  algo_timer = 0;
  log_event("ALGORITHM engaged: squad A inbound");
}

// Advances the scripted choreography by one frame. Returns a status message
// when the phase changes, or NULL if nothing changed this frame.
const char *algo_tick(void) {
  if (algo_phase == 1) {
    if (turnstile_crossings - algo_baseline >= ALGO_TRIGGER) {
      spawn_wave(1); // squad B: forward, from the right -- timed to
                     // meet squad A's now-inverted survivors head-on
      algo_phase = 2;
      log_event("ALGORITHM: squad B launched");
      return "ALGORITHM: squad B launched -- forward meets inverted";
    }
  } else if (algo_phase == 2) {
    algo_timer++;
    if (algo_timer >= ALGO_RUN_FRAMES) {
      algo_phase = 3;
      log_event("ALGORITHM complete");
      return "ALGORITHM complete";
    }
  }
  return NULL;
}