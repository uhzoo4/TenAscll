// img2art -- converts an image to braille-density terminal art with real
// 24-bit color. Standalone tool, own main(), no dependency on the TENET
// sim's platform layer -- behaves like a normal CLI tool (works piped/
// redirected, no TTY assumptions), so its rendering approach intentionally
// doesn't reuse render.c: photographic images need per-cell truecolor
// (millions of colors), not the sim's fixed 9-tier palette.
//
// Technique (same idea real terminal image viewers like chafa use):
//   1. Box-downsample the source image onto a SUBW x SUBH sub-pixel grid
//      (2x4 dots per character cell, same braille packing as the sim).
//   2. Floyd-Steinberg dither the luminance channel to 1-bit per dot --
//      this is what gives real shape/detail instead of flat grey blocks.
//   3. Color each character CELL with the average source RGB under it,
//      via a 24-bit truecolor ANSI escape (\x1b[38;2;R;G;Bm).
//
// Build:  gcc -O2 -Iinclude -o img2art src/img2art.c -lm
// Run:    ./img2art photo.jpg [output_width_in_chars]
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

#ifdef _WIN32
#include <windows.h>
#endif

static const int braille_bit[4][2] = {
    {0x01, 0x08}, {0x02, 0x10}, {0x04, 0x20}, {0x40, 0x80}};

int main(int argc, char **argv) {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8); // braille output is UTF-8
#endif

  if (argc < 2) {
    fprintf(stderr, "usage: %s <image> [output_width_in_chars]\n", argv[0]);
    return 1;
  }

  int src_w, src_h, src_ch;
  unsigned char *src = stbi_load(argv[1], &src_w, &src_h, &src_ch, 3);
  if (!src) {
    fprintf(stderr, "img2art: couldn't load '%s': %s\n", argv[1],
            stbi_failure_reason());
    return 1;
  }

  int cols = (argc >= 3) ? atoi(argv[2]) : 100;
  if (cols < 4)
    cols = 4;
  if (cols > 400)
    cols = 400; // sanity clamp -- terminals aren't infinite
  // braille dots are ~square in a typical monospace font (2 wide x 4 tall
  // sub-pixels inside a cell that's itself roughly 1 wide x 2 tall), so
  // the true image aspect ratio carries straight over with no correction
  int rows = (int)((double)cols * src_h / src_w / 2.0);
  if (rows < 2)
    rows = 2;
  int subw = cols * 2, subh = rows * 4;

  // --- pass 1: box-downsample source onto the sub-pixel luminance grid,
  //     and separately accumulate per-CELL average color ---
  float *lum = malloc(sizeof(float) * subw * subh);
  double *cell_r = calloc(cols * rows, sizeof(double));
  double *cell_g = calloc(cols * rows, sizeof(double));
  double *cell_b = calloc(cols * rows, sizeof(double));
  int *cell_n = calloc(cols * rows, sizeof(int));
  if (!lum || !cell_r || !cell_g || !cell_b || !cell_n) {
    fprintf(stderr, "img2art: out of memory\n");
    return 1;
  }

  for (int sy = 0; sy < subh; sy++) {
    int y0 = (int)((double)sy * src_h / subh);
    int y1 = (int)((double)(sy + 1) * src_h / subh);
    if (y1 <= y0)
      y1 = y0 + 1;
    if (y1 > src_h)
      y1 = src_h;
    for (int sx = 0; sx < subw; sx++) {
      int x0 = (int)((double)sx * src_w / subw);
      int x1 = (int)((double)(sx + 1) * src_w / subw);
      if (x1 <= x0)
        x1 = x0 + 1;
      if (x1 > src_w)
        x1 = src_w;

      long rs = 0, gs = 0, bs = 0, n = 0;
      for (int y = y0; y < y1; y++)
        for (int x = x0; x < x1; x++) {
          const unsigned char *px = src + (y * src_w + x) * 3;
          rs += px[0];
          gs += px[1];
          bs += px[2];
          n++;
        }
      double r = rs / (double)n, g = gs / (double)n, b = bs / (double)n;
      lum[sy * subw + sx] = (float)(0.299 * r + 0.587 * g + 0.114 * b);

      int cx = sx / 2, cy = sy / 4;
      cell_r[cy * cols + cx] += r;
      cell_g[cy * cols + cx] += g;
      cell_b[cy * cols + cx] += b;
      cell_n[cy * cols + cx] += 1;
    }
  }
  stbi_image_free(src);

  // --- pass 2: Floyd-Steinberg dither the luminance grid to 1-bit dots --
  //     this is what turns flat brightness into actual perceived detail
  unsigned char *dot = calloc(subw * subh, 1);
  for (int y = 0; y < subh; y++) {
    for (int x = 0; x < subw; x++) {
      float old = lum[y * subw + x];
      int on = old > 128.0f;
      dot[y * subw + x] = (unsigned char)on;
      float err = old - (on ? 255.0f : 0.0f);
      if (x + 1 < subw)
        lum[y * subw + x + 1] += err * 7.0f / 16.0f;
      if (y + 1 < subh && x > 0)
        lum[(y + 1) * subw + x - 1] += err * 3.0f / 16.0f;
      if (y + 1 < subh)
        lum[(y + 1) * subw + x] += err * 5.0f / 16.0f;
      if (y + 1 < subh && x + 1 < subw)
        lum[(y + 1) * subw + x + 1] += err * 1.0f / 16.0f;
    }
  }

  // --- pass 3: pack dots into braille codepoints + emit truecolor ANSI --
  size_t bufcap = (size_t)rows * (cols * 24 + 8) + 64;
  char *out = malloc(bufcap);
  size_t n = 0;
  int last_r = -1, last_g = -1, last_b = -1;

  for (int cy = 0; cy < rows; cy++) {
    for (int cx = 0; cx < cols; cx++) {
      unsigned char mask = 0;
      for (int dy = 0; dy < 4; dy++)
        for (int dx = 0; dx < 2; dx++) {
          int sx = cx * 2 + dx, sy = cy * 4 + dy;
          if (dot[sy * subw + sx])
            mask |= braille_bit[dy][dx];
        }

      int cn = cell_n[cy * cols + cx];
      int r = cn ? (int)(cell_r[cy * cols + cx] / cn) : 0;
      int g = cn ? (int)(cell_g[cy * cols + cx] / cn) : 0;
      int b = cn ? (int)(cell_b[cy * cols + cx] / cn) : 0;

      if (mask == 0) {
        r = 0;
        g = 0;
        b = 0;
      } // empty cell: don't recolor the terminal bg pointlessly
      if (r != last_r || g != last_g || b != last_b) {
        n += sprintf(out + n, "\x1b[38;2;%d;%d;%dm", r, g, b);
        last_r = r;
        last_g = g;
        last_b = b;
      }
      unsigned int cp = 0x2800 + mask;
      out[n++] = (char)(0xE0 | (cp >> 12));
      out[n++] = (char)(0x80 | ((cp >> 6) & 0x3F));
      out[n++] = (char)(0x80 | (cp & 0x3F));
    }
    out[n++] = '\n';
  }
  n += sprintf(out + n, "\x1b[0m");

  fwrite(out, 1, n, stdout);
  fflush(stdout);

  free(lum);
  free(dot);
  free(out);
  free(cell_r);
  free(cell_g);
  free(cell_b);
  free(cell_n);
  return 0;
}