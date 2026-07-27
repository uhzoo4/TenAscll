# TENASCII — TENET-style time-inversion particle playground

A terminal particle sim built around the actual film mechanic: particles
crossing a "turnstile" auto-invert and rewind their own recorded history,
and forward/inverted collisions are real paradox events (annihilation +
a fading scorch mark). Rendered in braille sub-pixels for higher-than-ASCII
resolution. Full design notes in `BLUEPRINT.md`.

## Layout

```
include/
  tenet.h        shared constants, Particle struct, all extern declarations
src/
  term.c         raw terminal mode (alt screen, non-blocking input)
  particle.c     physics, turnstile, annihilation, event log
  algorithm.c    the scripted two-squad pincer ("THE ALGORITHM")
  render.c       braille framebuffer, trail fade, turnstile ring, HUD
  main.c         entry point / input loop only
```

## Build

CMake:

```
cmake -B build
cmake --build build
./build/tenet        (or build\Debug\tenet.exe on Windows/MSVC generators)
```

Make:

```
make
./tenet
```

## Windows note

Run `chcp 65001` in your terminal before launching, or use Windows Terminal —
the braille rendering is UTF-8 and needs the console codepage to match.
`term.c` also sets this automatically on Windows builds.

## Controls

| Key   | Action |
|-------|--------|
| space | launch a forward wave from the left |
| p     | launch an unscripted pincer wave from the right |
| a     | run THE ALGORITHM (scripted, timed pincer) |
| i     | manual override — force-invert all forward particles |
| r     | reset |
| q     | quit |
