#include "platform.hpp"

Platform::Platform(char const *title, int windowWidth, int windowHeight,
                   int textureWidth, int textureHeight) {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(title, 0, 0, windowWidth, windowHeight,
                              SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                SDL_TEXTUREACCESS_STREAMING, textureWidth,
                                textureHeight);
}

Platform::~Platform() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Platform::Update(void const *buffer, int pitch) {
    SDL_UpdateTexture(texture, nullptr, buffer, pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

bool Platform::ProcessInput(uint8_t *keys) {
    bool quit = false;

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            quit = true;
            break;

        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            bool pressed = (event.type == SDL_KEYDOWN);
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                quit = true;
                break;
            case SDLK_x:
                keys[0] = pressed;
                break;
            case SDLK_1:
                keys[1] = pressed;
                break;
            case SDLK_2:
                keys[2] = pressed;
                break;
            case SDLK_3:
                keys[3] = pressed;
                break;
            case SDLK_q:
                keys[4] = pressed;
                break;
            case SDLK_w:
                keys[5] = pressed;
                break;
            case SDLK_e:
                keys[6] = pressed;
                break;
            case SDLK_a:
                keys[7] = pressed;
                break;
            case SDLK_s:
                keys[8] = pressed;
                break;
            case SDLK_d:
                keys[9] = pressed;
                break;
            case SDLK_z:
                keys[0xA] = pressed;
                break;
            case SDLK_c:
                keys[0xB] = pressed;
                break;
            case SDLK_4:
                keys[0xC] = pressed;
                break;
            case SDLK_r:
                keys[0xD] = pressed;
                break;
            case SDLK_f:
                keys[0xE] = pressed;
                break;
            case SDLK_v:
                keys[0xF] = pressed;
                break;
            }
            break;
        }
        }
    }

    return quit;
}
