#pragma once
#include <span>
#include <string_view>

#include "codotaku_utils.hpp"
#include "SDL3/SDL_render.h"
#include "SDL3_ttf/SDL_ttf.h"

class WindowContext {
public:
    struct Options {
        std::string_view title = "Codotaku App";
        int width = 800;
        int height = 600;
    };

    explicit WindowContext(const Options &opts) {
        chk(SDL_SetAppMetadata(opts.title.data(), "1.0", "com.codotaku.demo"));
        chk(SDL_Init(SDL_INIT_VIDEO));
        chk(TTF_Init());

        chk(SDL_CreateWindowAndRenderer(opts.title.data(), opts.width, opts.height,
                                        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN,
                                        &window_, &renderer_));

        chk(SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND));
        textEngine_ = TTF_CreateRendererTextEngine(renderer_);

        chk(SDL_GetWindowSize(window_, &width_, &height_));
    }

    ~WindowContext() {
        if (textEngine_) TTF_DestroyRendererTextEngine(textEngine_);
        if (renderer_) SDL_DestroyRenderer(renderer_);
        if (window_) SDL_DestroyWindow(window_);
        TTF_Quit();
    }

    // Explicitly non-copyable to prevent accidental asset destruction leaks
    WindowContext(const WindowContext &) = delete;

    WindowContext &operator=(const WindowContext &) = delete;

    [[nodiscard]] SDL_Window *window() const { return window_; }
    [[nodiscard]] SDL_Renderer *renderer() const { return renderer_; }
    [[nodiscard]] TTF_TextEngine *textEngine() const { return textEngine_; }

    [[nodiscard]] int width() const { return width_; }
    [[nodiscard]] int height() const { return height_; }

    void SyncDimensions() {
        chk(SDL_GetWindowSize(window_, &width_, &height_));
    }

    void Show() const {
        chk(SDL_ShowWindow(window_));
    }

    void Clear(SDL_FColor clearColor = {1.0f, 0.0f, 0.0f, 1.0f}) const {
        chk(SDL_SetRenderDrawColorFloat(renderer_, clearColor.r, clearColor.g, clearColor.b, clearColor.a));
        chk(SDL_RenderClear(renderer_));
    }

    void Present() const {
        chk(SDL_RenderPresent(renderer_));
    }

private:
    SDL_Window *window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;
    TTF_TextEngine *textEngine_ = nullptr;
    int width_ = 0;
    int height_ = 0;
};
