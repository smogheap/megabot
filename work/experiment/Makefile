SDL_CFLAGS := $(shell sdl-config --cflags)
SDL_LDFLAGS := $(shell sdl-config --libs)
MIXER_FLAGS := -lSDL_mixer -lpthread
CC = gcc

all: $(OBJS)
	$(CC) $(SDL_LDFLAGS) $(MIXER_FLAGS) playmusic.o -o playmusic
%.o: %.c
	$(CC) $(SDL_CFLAGS) -c -o $@ $<
