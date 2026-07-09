#include "SDL3/SDL_error.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"
#include <SDL3/SDL.h>

struct GameCTX {
    SDL_Renderer *renderer = nullptr;
    SDL_Window *window = nullptr;
};

void DrawFrame(GameCTX &ctx) {
    SDL_SetRenderDrawColorFloat(ctx.renderer, 1.0f, 1.0f, 1.0f, SDL_ALPHA_OPAQUE_FLOAT);

    SDL_RenderClear(ctx.renderer);
    SDL_RenderPresent(ctx.renderer);
}

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return 1;
    }

    GameCTX ctx;

    if (!SDL_CreateWindowAndRenderer("Snake", 1920, 1080, SDL_WINDOW_RESIZABLE, &ctx.window,
                                    &ctx.renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return 1;
    }

    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            }
        }
        DrawFrame(ctx);
    }

    SDL_DestroyWindow(ctx.window);
    SDL_Quit();

    return 0;
}