// Raw terminal mode: alt screen, hide cursor, no echo/canonical input,
// non-blocking reads. Fully self-contained — nothing else in the program
// touches termios.
//
// On Windows, the console's default OEM codepage isn't UTF-8, so the
// braille framebuffer (which is pure UTF-8) renders as mojibake unless we
// switch it. The #ifdef below is a no-op everywhere except native Windows
// builds (e.g. MSYS2/MinGW gcc) -- it does nothing on Linux/macOS.
#include "tenet.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
static void win_force_utf8(void) {
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
}
#endif

static struct termios orig_term;

static void term_restore(void) {
  tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);
  printf("\x1b[?25h\x1b[?1049l"); // show cursor, leave alt screen
  fflush(stdout);
}

void term_raw(void) {
#ifdef _WIN32
  win_force_utf8();
#endif
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