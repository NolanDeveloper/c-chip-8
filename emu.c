#include <stdio.h>
#include <time.h>

#include "SDL.h"

#include "chip8.h"

#define WINDOW_WIDTH (DISPLAY_WIDTH * 10)
#define WINDOW_HEIGHT (DISPLAY_HEIGHT * 10)

static unsigned running;
static SDL_Surface * window;

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
    running = 1;
    SDL_Rect rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    SDL_FillRect(window, &rect, 0x5d2600);
    unsigned last_tick = 0;
    unsigned last_display_update = 0;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            unsigned n;
            switch (event.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                case SDLK_1: n = 0x1; break;
                case SDLK_2: n = 0x2; break;
                case SDLK_3: n = 0x3; break;
                case SDLK_4: n = 0xc; break;
                case SDLK_q: n = 0x4; break;
                case SDLK_w: n = 0x5; break;
                case SDLK_e: n = 0x6; break;
                case SDLK_r: n = 0xd; break;
                case SDLK_a: n = 0x7; break;
                case SDLK_s: n = 0x8; break;
                case SDLK_d: n = 0x9; break;
                case SDLK_f: n = 0xe; break;
                case SDLK_z: n = 0xa; break;
                case SDLK_x: n = 0x0; break;
                case SDLK_c: n = 0xb; break;
                case SDLK_v: n = 0xf; break;
                default: goto break_inner_switch;
                }
                set_key_state(n, SDL_KEYDOWN == event.type ? 1u : 0u);
                break;
            case SDL_QUIT:
                running = 0;
                break;
            }
break_inner_switch:;
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
