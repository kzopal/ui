ui: ui.c ui.gif stb_image.h
	$(CC) -Os -s -Wl,--gc-sections -o $@ ui.c -lX11 -lXext -lm

clean:
	rm -f ui
