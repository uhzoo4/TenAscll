// TENET -- time-inversion particle playground. See tenet.h for the module
// layout: term.c (terminal), particle.c (physics/state), algorithm.c
// (scripted pincer), render.c (drawing). This file just wires them together.
//
// Controls:
//   SPACE  launch a forward wave from the left
//   p      launch an unscripted pincer wave from the right (forward)
//   a      run THE ALGORITHM -- the scripted, timed pincer
//   i      manual override -- force-invert every forward particle (debug)
//   r      reset everything
//   q      quit
//
// Build:  make            (or: gcc -O2 -o tenet *.c -lm)
// Run:    ./tenet          (real terminal, >= 110x44, UTF-8 + truecolor)
#include "tenet.h"
#include <time.h>
#include <unistd.h>

int main(void) {
  term_raw();
  CX = SUBW / 2.0;
  CY = SUBH / 2.0;
  reset_sim();
  spawn_wave(0);

  struct timespec ts = {0, 33 * 1000000L}; // ~30fps
  const char *msg = "temporal pincer movement ready";
  int running = 1;
  while (running) {
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1) {
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

    nanosleep(&ts, NULL);
  }
  return 0;
}