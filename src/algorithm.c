#include "../include/tenet.h"

int algo_phase = 0;
int algo_baseline = 0;
int algo_timer = 0;
const int ALGO_TRIGGER = 15;
const int ALGO_RUN_FRAMES = 240;

void algo_start(void) {
    reset_sim();
    spawn_wave(0); // Squad A: forward from left
    algo_phase = 1;
    algo_baseline = turnstile_crossings;
    algo_timer = 0;
}

const char *algo_tick(void) {
    if (algo_phase == 1) {
        if (turnstile_crossings - algo_baseline >= ALGO_TRIGGER) {
            spawn_wave(1); // Squad B: forward from right
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
