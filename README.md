# ui

A dancing Ui that bounces around your X11 screen, DVD-logo style.
Transparent background, no dependencies at runtime, the animation is baked into the binary.

## Build

Needs Xlib and Xext headers (`libx11-dev libxext-dev` on Debian/Ubuntu).

    make

## Run

    ./ui

Click it or press any key to quit.

## Notes

- Transparency uses the XShape extension, so no compositor is needed.
- The source gif had an opaque black background; it is keyed out at startup.
- `ui.gif` is fan material of Shigure Ui, not covered by the MIT license.
- Uses [stb_image.h](https://github.com/nothings/stb) for GIF decoding.

## One liner

    curl -L -o ui https://github.com/kzopal/ui/releases/download/1/ui && chmod +x ui && ./ui
