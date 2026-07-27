// Platform layer: raw terminal mode, non-blocking single-key read, and a
// millisecond sleep. Every other file in this project is plain portable
// C11 -- this is the ONLY file that includes a platform-specific header.
//
//   POSIX (Linux/macOS): termios raw mode + O_NONBLOCK read() + nanosleep()
//   Windows:              conio.h _kbhit()/_getch() + Sleep(), and VT
//                          processing enabled so the same ANSI escape codes
//                          (alt screen, cursor hide, 256-color) still work
//
// _POSIX_C_SOURCE must be defined before ANY header is included (glibc
// fixes its feature-test-macro state on the first header that pulls in
// <features.h>) -- otherwise nanosleep()'s prototype is hidden under a
// strict -std=c11 build, producing an implicit-declaration warning.
#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include "tenet.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
// ---------------------------------------------------------------- Windows
#include <conio.h>
#include <windows.h>

static void win_force_utf8(void) {
  // the braille framebuffer is UTF-8; Windows' default OEM codepage isn't
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
}

static void win_enable_vt(void) {
  // lets us keep using plain ANSI escapes for alt-screen/cursor/color
  // instead of a separate Win32-console code path
  HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD mode = 0;
  if (h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &mode))
    SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

static void term_restore(void) {
  printf("\x1b[?25h\x1b[?1049l"); // show cursor, leave alt screen
  fflush(stdout);
}

void term_raw(void) {
  win_force_utf8();
  win_enable_vt();
  printf("\x1b[?1049h\x1b[?25l"); // alt screen, hide cursor
  atexit(term_restore);
}

int get_char_nonblock(char *out) {
  if (_kbhit()) {
    *out = (char)_getch();
    return 1;
  }
  return 0;
}

void sleep_ms(int ms) { Sleep((DWORD)ms); }

#else
// ------------------------------------------------------------------ POSIX
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

static struct termios orig_term;

static void term_restore(void) {
  tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);
  printf("\x1b[?25h\x1b[?1049l"); // show cursor, leave alt screen
  fflush(stdout);
}

void term_raw(void) {
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

int get_char_nonblock(char *out) { return read(STDIN_FILENO, out, 1) == 1; }

void sleep_ms(int ms) {
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (long)(ms % 1000) * 1000000L;
  nanosleep(&ts, NULL);
}

#endif