#include "SDL3/SDL_error.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"
#include <SDL3/SDL.h>
#include <cmath>
#include <cstdint>
#include <iostream>

struct GameCTX {
    SDL_Renderer *renderer = nullptr;
    SDL_Window *window = nullptr;
    char *map = nullptr;
};

void InitMap(GameCTX &ctx, int size) {
    if (ctx.map != nullptr) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Map is not empty. Can't initalize.");
        return;
    }
    int mapSize = size * size;
    ctx.map = new char[mapSize];

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int index = (i * size) + j;

            if (i == 0 || i == size - 1) {
                ctx.map[index] = 'b';
            } else if (j == 0 || j == size - 1) {
                ctx.map[index] = 'b';
            } else {
                ctx.map[index] = '0';
            }
        }
    }

    int center = static_cast<int>(std::ceil(size / 2.0));
    int centerIndex = (center * size) + center;
    ctx.map[centerIndex] = 's';
}

void DebugPrintMap(GameCTX &ctx, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int index = (i * size) + j;
            std::cout << " " << ctx.map[index] << " ";
        }
        std::cout << "\n";
    }
}

void DestroyMap(GameCTX &ctx) {
    if (ctx.map == nullptr) {
        return;
    }

    delete[] ctx.map;
    ctx.map = nullptr;
}

void DrawFrame(GameCTX &ctx) {
    SDL_SetRenderDrawColorFloat(ctx.renderer, 1.0f, 1.0f, 1.0f, SDL_ALPHA_OPAQUE_FLOAT);

    SDL_RenderClear(ctx.renderer);
    SDL_RenderPresent(ctx.renderer);
}

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't initialize SDL: %s", SDL_GetError());
        return 1;
    }

    GameCTX ctx;

    if (!SDL_CreateWindowAndRenderer("Snake", 1920, 1080, SDL_WINDOW_RESIZABLE, &ctx.window,
                                     &ctx.renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't create window/renderer: %s", SDL_GetError());
        return 1;
    }

    bool isRunning = true;
    SDL_Event event;

    constexpr int MAP_SIZE = 40;
    InitMap(ctx, MAP_SIZE);
    DebugPrintMap(ctx, MAP_SIZE);

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            }
        }
        DrawFrame(ctx);
    }

    DestroyMap(ctx);

    SDL_DestroyWindow(ctx.window);
    SDL_Quit();

    return 0;
}