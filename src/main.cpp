#include "SDL3/SDL_error.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_filesystem.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_stdinc.h"
#include "SDL3/SDL_surface.h"
#include "SDL3/SDL_video.h"
#include <SDL3/SDL.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>

#define TEXTURE_BARRIER 0
#define TEXTURE_GROUND 1
#define TEXTURE_SNAKE 2
#define TEXTURE_FOOD 3

struct GameCTX {
    SDL_Renderer *renderer = nullptr;
    SDL_Window *window = nullptr;
    char *map = nullptr;
    int mapSize = 0;
    std::array<SDL_Texture *, 4> textures = {};
};

void InitMap(GameCTX &ctx, int size) {
    if (ctx.map != nullptr) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Map is not empty. Can't initalize.");
        return;
    }
    int mapSize = size * size;
    ctx.map = new char[mapSize];
    ctx.mapSize = size;

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int index = (i * size) + j;

            if (i == 0 || i == size - 1 || j == 0 || j == size - 1) {
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
    ctx.mapSize = 0;
}

SDL_Texture *LoadTexture(GameCTX &ctx, const char *fileName) {
    SDL_Texture *texture = nullptr;
    SDL_Surface *surface = nullptr;
    char *filePath = nullptr;

    SDL_asprintf(&filePath, "%s..\\resources\\%s", SDL_GetBasePath(), fileName);
    surface = SDL_LoadPNG(filePath);
    if (surface != nullptr) {
        SDL_LogWarn(SDL_LOG_CATEGORY_SYSTEM, "Couldn't load PNG file: %s", SDL_GetError());
    }

    SDL_free(filePath);

    texture = SDL_CreateTextureFromSurface(ctx.renderer, surface);
    if (texture != nullptr) {
        SDL_LogWarn(SDL_LOG_CATEGORY_SYSTEM, "Couldn't create static texture: %s", SDL_GetError());
    }

    SDL_DestroySurface(surface);
    return texture;
}

void DrawFrame(GameCTX &ctx) {
    SDL_SetRenderDrawColorFloat(ctx.renderer, 0.0f, 0.0f, 0.0f, SDL_ALPHA_OPAQUE_FLOAT);

    int screenWidth = 0;
    int screenHeight = 0;
    SDL_GetWindowSize(ctx.window, &screenWidth, &screenHeight);

    int mapSpace = screenWidth < screenHeight ? screenWidth : screenHeight;
    float scale = static_cast<float>(mapSpace) /
                  static_cast<float>(ctx.mapSize * ctx.textures[TEXTURE_GROUND]->h);

    float tileWidth = static_cast<float>(ctx.textures[TEXTURE_GROUND]->h) * scale;
    float tileHeight = static_cast<float>(ctx.textures[TEXTURE_GROUND]->w) * scale;

    SDL_FRect dst_rect;

    dst_rect.w = tileWidth;
    dst_rect.h = tileHeight;
    dst_rect.x = 0.0f;
    dst_rect.y = 0.0f;

    float xAxisOrigin =
        static_cast<float>(screenWidth) / 2.0f - static_cast<float>(ctx.mapSize) / 2.0f * tileWidth;

    float yAxisOrigin = static_cast<float>(screenHeight) / 2.0f -
                        static_cast<float>(ctx.mapSize) / 2.0f * tileHeight;

    SDL_RenderClear(ctx.renderer);

    for (int i = 0; i < ctx.mapSize; i++) {
        for (int j = 0; j < ctx.mapSize; j++) {
            dst_rect.x = xAxisOrigin + static_cast<float>(i) * tileWidth;
            dst_rect.y = yAxisOrigin + static_cast<float>(j) * tileHeight;

            int tileIndex = (i * ctx.mapSize) + j;

            switch (ctx.map[tileIndex]) {
            case 'b':
                SDL_RenderTexture(ctx.renderer, ctx.textures[TEXTURE_BARRIER], nullptr, &dst_rect);
                break;
            case '0':
                SDL_RenderTexture(ctx.renderer, ctx.textures[TEXTURE_GROUND], nullptr, &dst_rect);
                break;
            case 's':
                SDL_RenderTexture(ctx.renderer, ctx.textures[TEXTURE_SNAKE], nullptr, &dst_rect);
                break;
            case 'f':
                SDL_RenderTexture(ctx.renderer, ctx.textures[TEXTURE_FOOD], nullptr, &dst_rect);
                break;
            default:
                break;
            }
        }
    }

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

    ctx.textures[TEXTURE_BARRIER] = LoadTexture(ctx, "barrier.png");
    ctx.textures[TEXTURE_GROUND] = LoadTexture(ctx, "ground.png");
    ctx.textures[TEXTURE_SNAKE] = LoadTexture(ctx, "snake.png");
    ctx.textures[TEXTURE_FOOD] = LoadTexture(ctx, "food.png");

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            }
        }
        DrawFrame(ctx);
    }

    DestroyMap(ctx);

    for (auto *texture : ctx.textures) {
        if (texture != nullptr)
            SDL_DestroyTexture(texture);
    }
    SDL_DestroyWindow(ctx.window);
    SDL_Quit();

    return 0;
}