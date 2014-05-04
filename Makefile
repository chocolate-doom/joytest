
CFLAGS = $(shell sdl-config --cflags)
LDFLAGS = $(shell sdl-config --libs)

joytest : joytest.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

