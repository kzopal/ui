/* Bouncing dancing Ui for X11. Click or press a key to quit.
 * Build: gcc -Os -s -Wl,--gc-sections -o ui ui.c -lX11 -lXext -lm */
#include <X11/Xlib.h>
#include <X11/extensions/shape.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_GIF
#include "stb_image.h"

__asm__(".section .rodata\n.global gs,ge\ngs:\n.incbin \"ui.gif\"\nge:\n.previous");
extern const unsigned char gs[], ge[];

int main(void) {
    int W, H, n, c, *dl;
    unsigned char *px = stbi_load_gif_from_memory(gs, ge - gs, &dl, &W, &H, &n, &c, 4);
    Display *d = XOpenDisplay(0);
    if (!px || !d) return 1;
    int s = DefaultScreen(d), sw = DisplayWidth(d, s), sh = DisplayHeight(d, s);
    int dp = DefaultDepth(d, s), row = (W + 7) / 8, *st = malloc(W * H * sizeof *st);
    Visual *v = DefaultVisual(d, s);

    XSetWindowAttributes a = {.override_redirect = 1, .event_mask = ButtonPressMask | KeyPressMask};
    Window w = XCreateWindow(d, RootWindow(d, s), 0, 0, W, H, 0, dp, InputOutput, v,
                             CWOverrideRedirect | CWEventMask, &a);
    GC gc = XCreateGC(d, w, 0, 0);
    Pixmap *img = malloc(n * sizeof *img), *msk = malloc(n * sizeof *msk);

    for (int f = 0; f < n; f++) {
        unsigned char *q = px + (long)f * W * H * 4, *bits = calloc(row, H);
        /* bg is opaque black: clear near-black pixels connected to the border */
        int sp = 0;
        #define PUSH(i) { unsigned char *p = q + (i) * 4; \
            if (p[3] && p[0] + p[1] + p[2] <= 24) { p[3] = 0; st[sp++] = (i); } }
        for (int x = 0; x < W; x++) { PUSH(x); PUSH((H - 1) * W + x); }
        for (int y = 0; y < H; y++) { PUSH(y * W); PUSH(y * W + W - 1); }
        while (sp) {
            int i = st[--sp], x = i % W, y = i / W;
            if (x) PUSH(i - 1);
            if (x < W - 1) PUSH(i + 1);
            if (y) PUSH(i - W);
            if (y < H - 1) PUSH(i + W);
        }
        XImage *im = XCreateImage(d, v, dp, ZPixmap, 0, malloc(W * H * 4), W, H, 32, 0);
        for (int i = 0; i < W * H; i++) {
            unsigned char *p = q + i * 4;
            XPutPixel(im, i % W, i / W, p[0] << 16 | p[1] << 8 | p[2]);
            if (p[3] >= 128) bits[i / W * row + i % W / 8] |= 1 << (i % W % 8);
        }
        img[f] = XCreatePixmap(d, w, W, H, dp);
        XPutImage(d, img[f], gc, im, 0, 0, 0, 0, W, H);
        msk[f] = XCreateBitmapFromData(d, w, (char *)bits, W, H);
        XDestroyImage(im);
        free(bits);
    }

    srand(time(0));
    int x = rand() % (sw - W), y = rand() % (sh - H), dx = 3, dy = 3, f = -1;
    long next = 0;
    XMapRaised(d, w);
    for (;;) {
        while (XPending(d)) {
            XEvent e; XNextEvent(d, &e);
            if (e.type == ButtonPress || e.type == KeyPress) return 0;
        }
        struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t);
        long ms = t.tv_sec * 1000 + t.tv_nsec / 1000000;
        if (ms >= next) {
            f = (f + 1) % n;
            next = ms + (dl[f] > 0 ? dl[f] : 40);
            XShapeCombineMask(d, w, ShapeBounding, 0, 0, msk[f], ShapeSet);
            XSetWindowBackgroundPixmap(d, w, img[f]);
            XClearWindow(d, w);
        }
        x += dx; y += dy;
        if (x <= 0 || x >= sw - W) dx = -dx;
        if (y <= 0 || y >= sh - H) dy = -dy;
        XMoveWindow(d, w, x, y);
        XRaiseWindow(d, w);
        XFlush(d);
        usleep(16000);
    }
}
