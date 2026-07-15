#include "SDL3/SDL_error.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_filesystem.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_scancode.h"
#include "SDL3/SDL_stdinc.h"
#include "SDL3/SDL_surface.h"
#include "SDL3/SDL_timer.h"
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

enum Direction { Up, Down, Left, Right };

struct SnakeBodyPart {
    SnakeBodyPart *nextBodyPart = nullptr;
    int x = 0;
    int y = 0;
};

struct GameCTX {
    SDL_Renderer *renderer = nullptr;
    SDL_Window *window = nullptr;
    double deltaTime = 0.0;

    char *map = nullptr;
    int mapSize = 0;

    std::array<SDL_Texture *, 4> textures = {};

    SnakeBodyPart *snakeHead = nullptr;
    SnakeBodyPart *snakeTail = nullptr;
    Direction snakeDirection = Direction::Up;
    double snakeSpeed = 175.0 / 1000.0; // miliseconds to move 1 field
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
    if (ctx.snakeHead != nullptr) {
        ctx.snakeHead->x = center;
        ctx.snakeHead->y = center;
    }
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

void SpawnFood(GameCTX &ctx) {
    int index = 0;
    int x = rand() % ctx.mapSize;
    int y = rand() % ctx.mapSize;
    index = (x * ctx.mapSize) + y;

    while(ctx.map[index] != '0') {
        x = rand() % ctx.mapSize;
        y = rand() % ctx.mapSize;
        index = (x * ctx.mapSize) + y;
    }

    ctx.map[index] = 'f';
}

void UpdateSnake(GameCTX &ctx) {
    int checkXDir = 0;
    int checkYDir = 0;

    switch (ctx.snakeDirection) {
    case Direction::Up:
        checkXDir = 0;
        checkYDir = -1;
        break;
    case Direction::Down:
        checkXDir = 0;
        checkYDir = 1;
        break;
    case Direction::Left:
        checkXDir = -1;
        checkYDir = 0;
        break;
    case Direction::Right:
        checkXDir = 1;
        checkYDir = 0;
        break;
    }

    int checkX = ctx.snakeHead->x + checkXDir;
    int checkY = ctx.snakeHead->y + checkYDir;

    int checkIndex = (checkX * ctx.mapSize) + checkY;
    int oldPosIndex = (ctx.snakeTail->x * ctx.mapSize) + ctx.snakeTail->y;

    switch (ctx.map[checkIndex]) {
    case 'b':
        break;
    case 'f':
        SpawnFood(ctx);
    case '0':
        ctx.map[checkIndex] = 's';
        ctx.map[oldPosIndex] = '0';
        ctx.snakeHead->x = checkX;
        ctx.snakeHead->y = checkY;
        break;
    case 's':
        break;
    default:
        break;
    }
}

void DestroySnake(GameCTX &ctx) {
    SnakeBodyPart *currentPart = ctx.snakeHead;
    SnakeBodyPart *nextPart = nullptr;
    while (currentPart != nullptr) {
        nextPart = currentPart->nextBodyPart;
        delete currentPart;
        currentPart = nextPart;
    }
    ctx.snakeHead = nullptr;
}

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't initialize SDL: %s", SDL_GetError());
        return 1;
    }
    srand(time(nullptr));
    GameCTX ctx;

    if (!SDL_CreateWindowAndRenderer("Snake", 960, 540, SDL_WINDOW_RESIZABLE, &ctx.window,
                                     &ctx.renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't create window/renderer: %s", SDL_GetError());
        return 1;
    }

    bool isRunning = true;
    SDL_Event event;

    ctx.snakeHead = new SnakeBodyPart;
    ctx.snakeTail = ctx.snakeHead;

    constexpr int MAP_SIZE = 40;
    InitMap(ctx, MAP_SIZE);
    DebugPrintMap(ctx, MAP_SIZE);
    SpawnFood(ctx);

    ctx.textures[TEXTURE_BARRIER] = LoadTexture(ctx, "barrier.png");
    ctx.textures[TEXTURE_GROUND] = LoadTexture(ctx, "ground.png");
    ctx.textures[TEXTURE_SNAKE] = LoadTexture(ctx, "snake.png");
    ctx.textures[TEXTURE_FOOD] = LoadTexture(ctx, "food.png");

    Uint64 lastCounter = SDL_GetPerformanceCounter();
    double moveCounter = 0.0;

    while (isRunning) {
        Uint64 currentCounter = SDL_GetPerformanceCounter();
        ctx.deltaTime = static_cast<double>(currentCounter - lastCounter) /
                        static_cast<double>(SDL_GetPerformanceFrequency());
        lastCounter = currentCounter;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                switch (event.key.scancode) {
                    case SDL_SCANCODE_UP:
                        if (ctx.snakeDirection != Direction::Down)
                            ctx.snakeDirection = Direction::Up;
                    break;
                    case SDL_SCANCODE_DOWN:
                        if (ctx.snakeDirection != Direction::Up)
                            ctx.snakeDirection = Direction::Down;
                    break;
                    case SDL_SCANCODE_LEFT:
                        if (ctx.snakeDirection != Direction::Right)
                            ctx.snakeDirection = Direction::Left;
                    break;
                    case SDL_SCANCODE_RIGHT:
                        if (ctx.snakeDirection != Direction::Left)
                            ctx.snakeDirection = Direction::Right;
                    break;
                    default: break;
                }
            }
        }

        moveCounter += ctx.deltaTime;
        if (moveCounter >= ctx.snakeSpeed) {
            moveCounter -= ctx.snakeSpeed;
            UpdateSnake(ctx);
        }

        DrawFrame(ctx);
    }

    DestroySnake(ctx);
    DestroyMap(ctx);

    for (auto *texture : ctx.textures) {
        if (texture != nullptr)
            SDL_DestroyTexture(texture);
    }
    SDL_DestroyWindow(ctx.window);
    SDL_Quit();

    return 0;
}