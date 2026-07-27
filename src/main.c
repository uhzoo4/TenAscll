#include "../include/tenet.h"

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
            if (c == 'q') {
                running = 0;
            } else if (c == ' ') {
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
            if (am) msg = am;
        }

        int fc = 0, ic = 0, rc = 0;
        for (int i = 0; i < MAX_PART; i++) {
            if (P[i].state == 1) fc++;
            else if (P[i].state == 2) ic++;
            else if (P[i].state == 3) rc++;
        }
        draw(fc, ic, rc, msg);

        sleep_ms(33);
    }
    return 0;
}
