#include <stdio.h>
#include <time.h>

#include "SDL.h"

#include "chip8.h"
#include "utils.h"

#define WINDOW_WIDTH (DISPLAY_WIDTH * 10)
#define WINDOW_HEIGHT (DISPLAY_HEIGHT * 10)

static unsigned running;
static SDL_Surface * window;

/*
    Chip8       Keyboard
    1 2 3 c     1 2 3 4
    4 5 6 d     q w e r
    7 8 9 e     a s d f
    a 0 b f     z x c v
*/
static SDLKey keymap[] = {
    SDLK_x, SDLK_1, SDLK_2, SDLK_3,
    SDLK_q, SDLK_w, SDLK_e, SDLK_a,
    SDLK_s, SDLK_d, SDLK_z, SDLK_c,
    SDLK_4, SDLK_r, SDLK_f, SDLK_v,
};

static void
update_display() {
    SDL_Rect rect;
    rect.w = rect.h = 9;
    for (int row = 0; row < DISPLAY_HEIGHT; ++row) {
        rect.y = 10 * row;
        for (int col = 0; col < DISPLAY_WIDTH; ++col) {
            rect.x = 10 * col;
            SDL_FillRect(window, &rect,
                get_pixel(row, col) ? 0xddb500 : 0x6d3600);
        }
    }
    SDL_Flip(window);
}

int main(int argc, char * argv[]) {
    srand(time(NULL));
    if (argc < 2) {
        printf("Filename must be provided.\n");
        return -1;
    }
    reset_ram();
    if (load_file(argv[1])) return -1;
    if (-1 == SDL_Init(SDL_INIT_VIDEO)) return -1;
    window = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_DOUBLEBUF);
    if (!window) return -1;
    running = 1;
    SDL_Rect rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    SDL_FillRect(window, &rect, 0x5d2600);
    unsigned last_tick = 0;
    unsigned last_display_update = 0;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                for (unsigned i = 0; i < 16; ++i) {
                    if (event.key.keysym.sym != keymap[i]) continue;
                    set_key_state(i, SDL_KEYDOWN == event.type ? 1 : 0);
                    goto break_inner;
                }
                break;
            case SDL_QUIT:
                running = 0;
                break;
            }
break_inner:;
        }
        unsigned now = SDL_GetTicks();
        if (now - last_tick > 1) {
            tick();
            last_tick = now;
        }
        if (now - last_display_update > 30) {
            update_display();
            last_display_update = now;
        }
    }
    SDL_FreeSurface(window);
    SDL_Quit();
}
