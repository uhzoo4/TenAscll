# TENET — Terminal Time-Inversion Playground
### Planning & Architecture Blueprint

No execution until you say go. This is the full plan: theme, rendering method, data structures, phases, and stretch goals.

---

## 1. Theme

Straight from the film's actual internal logic, not a vague "reverse video" gimmick:

| Concept in film | Concept in sim |
|---|---|
| Entropy inversion (object moves backward through *its own* recorded life) | Each particle keeps a growing history buffer of every position it has occupied. "Inverting" it doesn't compute new reverse-physics — it just **replays the buffer backward, frame by frame** |
| Blue = forward people, Red = inverted people | Blue particles simulated forward under gravity; Red particles in playback mode |
| Turnstile (the machine that flips your entropy) | A region in space — a ring, a doorway, or the whole singularity — that flips a particle's state the instant it's crossed |
| Pincer movement (forward squad + inverted squad converging from opposite directions in time) | Two waves on screen at once, one blue advancing, one red rewinding, sharing the same physical space but opposite temporal direction |
| Paradox / algorithm | Any blue+red particle in the same cell at the same moment → flash event |
| "What's happened, happened" (inevitability) | Once a particle is inverted, its path is **not recalculated** — it is fixed, since it already happened. This is a deliberate design constraint, not a limitation: inverted particles are read-only |

Visual palette: ANSI 256-color — blue `38;5;39`, red `38;5;196`, paradox white `38;5;231`, idle grid grey `38;5;238`. This mirrors the movie's actual color grading for forward/inverted operatives.

---

## 2. ASCII/Braille Rendering — how the picture is actually made

Plain ASCII gives 1 "pixel" per character cell — too chunky for smooth gravitational arcs. Instead:

- **Unicode braille characters** (`U+2800`–`U+28FF`) each encode an independent **2×4 grid of dots** (8 sub-pixels) via 8 bits of the codepoint.
- So one terminal character cell = 8 addressable sub-pixels. A terminal of `110×42` characters becomes an effective simulation canvas of `220×168` sub-pixels — a ~4x resolution multiplier over plain ASCII, for free.
- Bit layout used (standard braille dot numbering):

  ```
  (0,0)=0x01   (1,0)=0x08
  (0,1)=0x02   (1,1)=0x10
  (0,2)=0x04   (1,2)=0x20
  (0,3)=0x40   (1,3)=0x80
  ```
  where `(sub_x, sub_y)` is the dot's position inside its 2×4 cell.

- **Two parallel buffers** per frame:
  - `dotmask[row][col]` — which of the 8 sub-pixel bits are lit (built by OR-ing in bits as particles are plotted)
  - `colorbuf[row][col]` — which color "owns" that character cell, resolved by priority (paradox white > inverted red > forward blue), since a whole character cell can only carry one foreground color in a terminal even though it holds 8 independent dots
- Plotting a particle: convert its float `(x, y)` sub-pixel coordinate → `(char_col, char_row)` + `(bit_x, bit_y)` inside that cell → OR the corresponding bit into `dotmask`, update `colorbuf` if this color outranks what's already there.
- Frame present: for each cell, emit `0x2800 + dotmask[cell]` as a UTF-8 codepoint, switching the ANSI color escape only when the color actually changes between adjacent cells (avoids re-emitting an escape code per character — cuts terminal write volume a lot).

This is the same trick used by terminal image previewers (`chafa`, `viu`) and terminal Mandelbrot/plasma demos — braille as a cheap high-density framebuffer.

---

## 3. Core Data Structures

```c
typedef struct {
    double x, y, vx, vy;      // sub-pixel space position/velocity
    double *hx, *hy;          // recorded history (grows via realloc doubling)
    int hlen, hcap;
    int hplay;                 // playback cursor when inverted
    int state;                 // 0 dead / 1 forward / 2 inverted / 3 returned-frozen
} Particle;
```

- **Forward particles**: physics integrates them (gravity toward a central singularity + wall bounce with restitution), and every step appends `(x,y)` to `hx/hy`.
- **Inverted particles**: no physics at all — `hplay` decrements each frame, `x,y` are just read back out of `hx[hplay], hy[hplay]`. When `hplay` hits 0, particle → `state 3` (frozen, "returned to origin").
- **Collision pass**: O(n²) pairwise check between all live particles (fine at a few hundred particles — no spatial partitioning needed at this scale) flags any forward/inverted pair within a small radius as a paradox event, which lights up that character cell white for a few frames.

---

## 4. Implementation Phases

**Phase 0 — Skeleton (done previously as v0)**
Braille framebuffer, raw terminal mode, particle struct, forward gravity, spawn/reset. *(This already exists from the earlier build — treat it as the base to iterate on, not redo.)*

**Phase 1 — Turnstile mechanic**
Replace manual `i` key (invert-all) with a **spatial turnstile**: a defined region (ring around singularity, or a vertical "doorway" line) that auto-flips any particle's state the instant its center crosses it. This is the actual film mechanic instead of a global toggle.

**Phase 2 — Proper pincer choreography**
Scripted wave timing: wave A launches, crosses turnstile and inverts partway through its arc, wave B launches afterward and remains forward — timed so the two converge on the same region at the same moment, producing a real "pincer" rather than a coincidental overlap.

**Phase 3 — Paradox consequences**
Right now a collision just flashes. Give it a mechanic: e.g., colliding particles could **swap velocities**, **annihilate**, or **freeze both permanently** — pick one rule and make it visually legible (a distinct color hold, not just a single-frame flash).

**Phase 4 — Polish pass**
- Velocity-based particle brightness/trail fade (older history points dimmer)
- On-screen event log line ("Paradox at wave 3", "12 particles returned")
- Optional: read terminal size dynamically (`ioctl TIOCGWINSZ`) instead of fixed `110×42`, so it fills whatever terminal you're in

**Phase 5 (stretch)** — Free-standing "Algorithm" mode: a scripted, non-interactive replay (like a movie beat) that runs a fixed choreographed pincer sequence start to finish, for showing off rather than playing with.

---

## 5. What's explicitly *not* in scope (keep it crackable)

- No spatial partitioning / quadtree — particle counts here (≤400) don't need it
- No physically rigorous "grandfather paradox" resolution logic — the paradox event is a visual flag, not a simulation-breaking rule
- No config file / CLI flags — hardcoded constants, tweak in source

---

Say the word and I'll build Phase 1 (turnstile) first, since it's the one change that turns this from "toy with a keybind" into the real mechanic.
