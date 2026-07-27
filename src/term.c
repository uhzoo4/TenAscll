#include "../include/tenet.h"

#ifdef _WIN32
#include <windows.h>
#include <conio.h>

static HANDLE hStdin, hStdout;
static DWORD orig_in_mode, orig_out_mode;

void term_restore(void) {
    SetConsoleMode(hStdin, orig_in_mode);
    SetConsoleMode(hStdout, orig_out_mode);
    printf("\x1b[?25h\x1b[?1049l"); // show cursor, leave alt screen
    fflush(stdout);
}

void term_raw(void) {
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleMode(hStdin, &orig_in_mode);
    GetConsoleMode(hStdout, &orig_out_mode);

    DWORD raw_in = orig_in_mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
    SetConsoleMode(hStdin, raw_in);

    DWORD raw_out = orig_out_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hStdout, raw_out);

    printf("\x1b[?1049h\x1b[?25l"); // alt screen, hide cursor
    atexit(term_restore);
}

int get_char_nonblock(char *c) {
    if (_kbhit()) {
        *c = (char)_getch();
        return 1;
    }
    return 0;
}

void sleep_ms(int ms) {
    Sleep(ms);
}

#else
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

static struct termios orig_term;

void term_restore(void) {
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

int get_char_nonblock(char *c) {
    return read(STDIN_FILENO, c, 1) == 1;
}

void sleep_ms(int ms) {
    struct timespec ts = {0, ms * 1000000L};
    nanosleep(&ts, NULL);
}
#endif
