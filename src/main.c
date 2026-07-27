#include "../include/tenascii.h"

int main(void) {
    print_banner();

    // Create a 60x12 ASCII canvas
    AsciiCanvas *canvas = create_canvas(60, 12);
    if (!canvas) {
        fprintf(stderr, "Failed to allocate ASCII canvas.\n");
        return 1;
    }

    // Draw outer box
    draw_box(canvas, 0, 0, 60, 12);

    // Draw inner box
    draw_box(canvas, 4, 2, 52, 8);

    // Render text inside
    draw_text(canvas, 14, 4, "Welcome to TENASCII Engine!");
    draw_text(canvas, 10, 6, "Built with C and MinGW Toolchain");
    draw_text(canvas, 18, 8, "Status: Operational [OK]");

    // Display formatted canvas with color styling
    printf(COLOR_GREEN);
    render_canvas(canvas);
    printf(COLOR_RESET);

    free_canvas(canvas);
    return 0;
}
