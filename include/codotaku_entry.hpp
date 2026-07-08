#pragma once

#ifndef SDL_MAIN_USE_CALLBACKS
#define SDL_MAIN_USE_CALLBACKS 1
#endif

#include <span>
#include <exception>

#include "SDL3/SDL_init.h"
#include "SDL3/SDL_main.h"
#include "SDL3/SDL_log.h"

class App;

inline auto SDL_AppInit(void **appstate, int argc, char *argv[]) -> SDL_AppResult try {
    auto app = std::make_unique<App>(std::span{argv, static_cast<size_t>(argc)});
    *appstate = app.release();
    return SDL_APP_CONTINUE;
} catch (const std::exception &e) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Initialization failed: %s", e.what());
    return SDL_APP_FAILURE;
}

inline auto SDL_AppEvent(void *appstate, SDL_Event *event) -> SDL_AppResult try {
    auto *app = static_cast<App *>(appstate);
    return app->Event(event);
} catch (const std::exception &e) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Event error: %s", e.what());
    return SDL_APP_FAILURE;
}

inline auto SDL_AppIterate(void *appstate) -> SDL_AppResult try {
    auto *app = static_cast<App *>(appstate);
    return app->Iterate();
} catch (const std::exception &e) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Iterate error: %s", e.what());
    return SDL_APP_FAILURE;
}

inline void SDL_AppQuit(void *appstate, SDL_AppResult result) try {
    if (appstate) {
        delete static_cast<App *>(appstate);
    }
} catch (const std::exception &e) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Shutdown error: %s", e.what());
}
