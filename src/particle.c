#include "../include/tenet.h"

// Global particle simulation state
Particle P[MAX_PART];
int flash_frames[ROWS][COLS];
double CX_POS, CY_POS;
double G = 1400.0;
double TURNSTILE_R = 34.0;
int turnstile_crossings = 0;
int paradox_events = 0;

void hist_push(Particle *p, double x, double y) {
    if (p->hlen >= p->hcap) {
        p->hcap = p->hcap ? p->hcap * 2 : 64;
        p->hx = realloc(p->hx, sizeof(double) * p->hcap);
        p->hy = realloc(p->hy, sizeof(double) * p->hcap);
    }
    p->hx[p->hlen] = x;
    p->hy[p->hlen] = y;
    p->hlen++;
}

void spawn_wave(int from_right) {
    for (int i = 0; i < MAX_PART; i++) {
        if (P[i].state != 0) continue;
        int n_to_spawn = 40;
        for (int k = 0; k < n_to_spawn && i < MAX_PART; k++, i++) {
            if (P[i].state != 0) continue;
            Particle *p = &P[i];
            free(p->hx); free(p->hy);
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

void invert_all_forward(void) {
    for (int i = 0; i < MAX_PART; i++) {
        if (P[i].state == 1 && P[i].hlen > 1) {
            P[i].state = 2;
            P[i].hplay = P[i].hlen - 1;
        }
    }
}

void reset_sim(void) {
    for (int i = 0; i < MAX_PART; i++) {
        free(P[i].hx); free(P[i].hy);
        memset(&P[i], 0, sizeof(P[i]));
    }
    memset(flash_frames, 0, sizeof(flash_frames));
    turnstile_crossings = 0;
    paradox_events = 0;
    algo_phase = 0;
    algo_baseline = 0;
    algo_timer = 0;
}

void physics_step(double dt) {
    for (int i = 0; i < MAX_PART; i++) {
        Particle *p = &P[i];
        if (p->state == 0 || p->state == 3) continue;

        if (p->state == 1) {
            // Forward: gravity toward singularity + wall bounce
            double dx0 = CX_POS - p->x, dy0 = CY_POS - p->y;
            double dist_before = sqrt(dx0 * dx0 + dy0 * dy0);

            double r2 = dx0 * dx0 + dy0 * dy0 + 400.0;
            double r = sqrt(r2);
            double f = G / r2;
            p->vx += f * dx0 / r * dt;
            p->vy += f * dy0 / r * dt;
            p->x += p->vx * dt;
            p->y += p->vy * dt;

            if (p->x < 1) { p->x = 1; p->vx = fabs(p->vx) * 0.85; }
            if (p->x > SUBW - 2) { p->x = SUBW - 2; p->vx = -fabs(p->vx) * 0.85; }
            if (p->y < 1) { p->y = 1; p->vy = fabs(p->vy) * 0.85; }
            if (p->y > SUBH - 2) { p->y = SUBH - 2; p->vy = -fabs(p->vy) * 0.85; }
            hist_push(p, p->x, p->y);

            // Turnstile: crossing INTO the ring auto-inverts entropy
            double dx1 = CX_POS - p->x, dy1 = CY_POS - p->y;
            double dist_after = sqrt(dx1 * dx1 + dy1 * dy1);
            if (dist_before > TURNSTILE_R && dist_after <= TURNSTILE_R && p->hlen > 1) {
                p->state = 2;
                p->hplay = p->hlen - 1;
                turnstile_crossings++;
            }
        } else if (p->state == 2) {
            // Inverted: walk backward through recorded trajectory
            if (p->hplay <= 0) {
                p->state = 3; // Returned to origin (frozen)
                continue;
            }
            p->hplay--;
            p->x = p->hx[p->hplay];
            p->y = p->hy[p->hplay];
        }
    }

    // Paradox Collision Detection: forward vs inverted sharing cell
    for (int i = 0; i < MAX_PART; i++) {
        if (P[i].state != 1 && P[i].state != 2) continue;
        for (int j = i + 1; j < MAX_PART; j++) {
            if (P[i].state != 1 && P[i].state != 2) break;
            if (P[j].state != 1 && P[j].state != 2) continue;
            if (P[i].state == P[j].state) continue;

            double dx = P[i].x - P[j].x, dy = P[i].y - P[j].y;
            if (dx * dx + dy * dy < 9.0) {
                int cx = (int)(P[i].x / 2), cy = (int)(P[i].y / 4);
                if (cx >= 0 && cx < COLS && cy >= 0 && cy < ROWS) {
                    if (flash_frames[cy][cx] == 0) paradox_events++;
                    flash_frames[cy][cx] = SCORCH_LIFETIME;
                }
                free(P[i].hx); free(P[i].hy); memset(&P[i], 0, sizeof(P[i]));
                free(P[j].hx); free(P[j].hy); memset(&P[j], 0, sizeof(P[j]));
            }
        }
    }
}
