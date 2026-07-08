#include <array>
#include <codotaku_utils.hpp>
#include <codotaku_window_context.hpp>
#include <functional> // For std::move_only_function
#include <memory_resource>
#include <span>

#include "SDL3/SDL_render.h"
#include "SDL3_ttf/SDL_ttf.h"
#include "codotaku_style_builder.hpp"
#include "yoga/YGConfig.h"
#include "yoga/YGNode.h"
#include "yoga/YGNodeLayout.h"


struct Widget {
    enum class Type {
        Container,
        Text
    };

    Type type = Type::Container;
    SDL_FColor color{0.0f, 0.0f, 0.0f, 0.0f};
    TTF_Text *text = nullptr;
    bool hovered = false;
    bool interactive = false;
    std::move_only_function<void()> onClick = nullptr;
};

static YGSize MeasureTextNode(YGNodeConstRef node, float width, YGMeasureMode widthMode, float height,
                              YGMeasureMode heightMode) {
    if (const auto *widget = static_cast<const Widget *>(YGNodeGetContext(node)); widget && widget->text) {
        if (widthMode != YGMeasureModeUndefined) {
            chk(TTF_SetTextWrapWidth(widget->text, static_cast<int>(width)));
        } else {
            chk(TTF_SetTextWrapWidth(widget->text, 0));
        }

        int textW = 0, textH = 0;
        if (TTF_GetTextSize(widget->text, &textW, &textH)) {
            return YGSize{static_cast<float>(textW), static_cast<float>(textH)};
        }
    }
    return YGSize{0, 0};
}

class App {
public:
    explicit App(std::span<char *> args)
        : ctx_({.title = "Codotaku Workspace", .width = 1024, .height = 768}) {
        auto base_path = SDL_GetBasePath();
        const auto font_path = std::format("{}{}", base_path, "assets/fonts/Inter_18pt-Regular.ttf");
        font_ = chk(TTF_OpenFont(font_path.c_str(), 14));

        config_ = YGConfigNew();
        const auto pixelsInPoint = chk(SDL_GetWindowDisplayScale(ctx_.window()));
        YGConfigSetPointScaleFactor(config_, pixelsInPoint);
        YGConfigSetUseWebDefaults(config_, true);

        // --- Root Canvas Window ---
        root_ = CreateContainer(nullptr, SDL_FColor{0.08f, 0.09f, 0.11f, 1.0f}, false, nullptr, [this](YGNodeRef node) {
            StyleBuilder(node)
                    .flexDirection(YGFlexDirectionColumn)
                    .width(ctx_.width())
                    .height(ctx_.height())
                    .padding(YGEdgeAll, 20.0f);
        });

        // --- Header Block Container ---
        const auto header = CreateContainer(root_, SDL_FColor{0.13f, 0.15f, 0.19f, 1.0f}, false, nullptr,
                                            [](YGNodeRef node) {
                                                StyleBuilder(node)
                                                        .flexDirection(YGFlexDirectionRow)
                                                        .widthPercent(100.0f)
                                                        .height(60.0f)
                                                        .padding(YGEdgeHorizontal, 16.0f)
                                                        .alignItems(YGAlignCenter)
                                                        .justifyContent(YGJustifySpaceBetween)
                                                        .margin(YGEdgeBottom, 20.0f);
                                            });
        CreateTextWidget(header, "WORKSPACE CONSOLE");

        const auto liveBadge = CreateContainer(header, SDL_FColor{0.18f, 0.45f, 0.23f, 1.0f}, false, nullptr,
                                               [](YGNodeRef node) {
                                                   StyleBuilder(node)
                                                           .padding(YGEdgeHorizontal, 12.0f)
                                                           .padding(YGEdgeVertical, 6.0f);
                                               });
        CreateTextWidget(liveBadge, "ONLINE");

        // --- Grid Matrix Row ---
        const auto gridRow = CreateContainer(root_, SDL_FColor{0.0f, 0.0f, 0.0f, 0.0f}, false, nullptr,
                                             [](YGNodeRef node) {
                                                 StyleBuilder(node)
                                                         .flexDirection(YGFlexDirectionRow)
                                                         .widthPercent(100.0f)
                                                         .gap(YGGutterAll, 16.0f)
                                                         .margin(YGEdgeBottom, 20.0f);
                                             });

        constexpr std::array metrics = {
            std::make_pair("Engine State", "Active / 60 FPS"),
            std::make_pair("Memory Heap", "Arena Balanced"),
            std::make_pair("Active Threads", "4 Cores allocated")
        };

        for (const auto &[title, data]: metrics) {
            const auto card = CreateContainer(gridRow, SDL_FColor{0.13f, 0.15f, 0.19f, 1.0f}, false, nullptr,
                                              [](YGNodeRef node) {
                                                  StyleBuilder(node)
                                                          .flexGrow(1.0f)
                                                          .flexBasis(0.0f)
                                                          .padding(YGEdgeAll, 16.0f)
                                                          .flexDirection(YGFlexDirectionColumn)
                                                          .gap(YGGutterAll, 4.0f);
                                              });
            CreateTextWidget(card, title);
            CreateTextWidget(card, data);
        }

        // --- Dynamic View Split ---
        const auto mainSplit = CreateContainer(root_, SDL_FColor{0.0f, 0.0f, 0.0f, 0.0f}, false, nullptr,
                                               [](YGNodeRef node) {
                                                   StyleBuilder(node)
                                                           .flexDirection(YGFlexDirectionRow)
                                                           .widthPercent(100.0f)
                                                           .flexGrow(1.0f)
                                                           .gap(YGGutterAll, 16.0f);
                                               });

        // Left Container Block: Logs Area
        const auto logsPane = CreateContainer(mainSplit, SDL_FColor{0.13f, 0.15f, 0.19f, 1.0f}, false, nullptr,
                                              [](YGNodeRef node) {
                                                  StyleBuilder(node)
                                                          .flexGrow(2.0f)
                                                          .flexBasis(0.0f)
                                                          .padding(YGEdgeAll, 20.0f)
                                                          .flexDirection(YGFlexDirectionColumn)
                                                          .gap(YGGutterAll, 12.0f);
                                              });
        CreateTextWidget(logsPane, "System Feed Logs");
        CreateTextWidget(
            logsPane,
            ">> Pure leaf components isolated successfully.\n>> Parent containers processing alignment properties independently.\n>> UI compilation complete.");

        // Right Container Block: Action Control Group
        const auto controlPane = CreateContainer(mainSplit, SDL_FColor{0.11f, 0.12f, 0.15f, 1.0f}, false, nullptr,
                                                 [](YGNodeRef node) {
                                                     StyleBuilder(node)
                                                             .flexGrow(1.0f)
                                                             .flexBasis(0.0f)
                                                             .padding(YGEdgeAll, 16.0f)
                                                             .flexDirection(YGFlexDirectionColumn)
                                                             .gap(YGGutterAll, 12.0f);
                                                 });
        CreateTextWidget(controlPane, "Actions Console");

        // Action 1
        const auto btnSync = CreateContainer(controlPane, SDL_FColor{0.22f, 0.25f, 0.32f, 1.0f}, true,
                                             []() { SDL_Log("Synchronizing Core Thread Channels..."); },
                                             [](YGNodeRef node) {
                                                 StyleBuilder(node)
                                                         .widthPercent(100.0f)
                                                         .height(44.0f)
                                                         .padding(YGEdgeLeft, 16.0f)
                                                         .alignItems(YGAlignCenter)
                                                         .justifyContent(YGJustifyFlexStart);
                                             });
        CreateTextWidget(btnSync, "Synchronize Thread");

        // Action 2
        const auto btnReset = CreateContainer(controlPane, SDL_FColor{0.22f, 0.25f, 0.32f, 1.0f}, true,
                                              []() { SDL_Log("Resetting Core Engine Layout Tree..."); },
                                              [](YGNodeRef node) {
                                                  StyleBuilder(node)
                                                          .widthPercent(100.0f)
                                                          .height(44.0f)
                                                          .padding(YGEdgeLeft, 16.0f)
                                                          .alignItems(YGAlignCenter)
                                                          .justifyContent(YGJustifyFlexStart);
                                              });
        CreateTextWidget(btnReset, "Reset Core Layout");

        ctx_.Show();
    }

    auto CreateContainer(YGNodeRef parent, SDL_FColor color, bool interactive, std::move_only_function<void()> onClick,
                         auto &&styling_callback) -> YGNodeRef {
        auto node = YGNodeNewWithConfig(config_);
        Widget *allocated = widget_allocator_.new_object<Widget>(Widget{
            .type = Widget::Type::Container,
            .color = color,
            .text = nullptr,
            .hovered = false,
            .interactive = interactive,
            .onClick = std::move(onClick)
        });
        YGNodeSetContext(node, allocated);

        if (parent) {
            YGNodeInsertChild(parent, node, YGNodeGetChildCount(parent));
        }
        styling_callback(node);
        return node;
    }

    auto CreateTextWidget(YGNodeRef parent, const char *content) -> YGNodeRef {
        const auto node = YGNodeNewWithConfig(config_);
        TTF_Text *textObj = chk(TTF_CreateText(ctx_.textEngine(), font_, content, 0));

        Widget *allocated = widget_allocator_.new_object<Widget>(Widget{
            .type = Widget::Type::Text,
            .color = {0.0f, 0.0f, 0.0f, 0.0f},
            .text = textObj,
            .hovered = false,
            .interactive = false,
            .onClick = nullptr
        });
        YGNodeSetContext(node, allocated);

        if (parent) {
            YGNodeInsertChild(parent, node, YGNodeGetChildCount(parent));
        }

        YGNodeSetMeasureFunc(node, MeasureTextNode);
        return node;
    }

    void RenderNode(YGNodeRef node, float parentX, float parentY) {
        if (!node) return;

        const float absX = parentX + YGNodeLayoutGetLeft(node);
        const float absY = parentY + YGNodeLayoutGetTop(node);
        const float width = YGNodeLayoutGetWidth(node);
        const float height = YGNodeLayoutGetHeight(node);

        if (const auto *widget = static_cast<Widget *>(YGNodeGetContext(node))) {
            if (widget->type == Widget::Type::Container) {
                const SDL_FRect rect{absX, absY, width, height};

                SDL_FColor renderColor = widget->color;
                if (widget->hovered && widget->color.a > 0.0f) {
                    renderColor.r = std::min(renderColor.r + 0.06f, 1.0f);
                    renderColor.g = std::min(renderColor.g + 0.06f, 1.0f);
                    renderColor.b = std::min(renderColor.b + 0.06f, 1.0f);
                }

                chk(SDL_SetRenderDrawColorFloat(ctx_.renderer(), renderColor.r, renderColor.g, renderColor.b,
                                                renderColor.a));
                chk(SDL_RenderFillRect(ctx_.renderer(), &rect));
            } else if (widget->type == Widget::Type::Text && widget->text) {
                chk(TTF_SetTextWrapWidth(widget->text, static_cast<int>(width)));
                chk(TTF_SetTextColorFloat(widget->text, 0.9f, 0.92f, 0.95f, 1.0f));
                chk(TTF_DrawRendererText(widget->text, absX, absY));
            }
        }

        const uint32_t childCount = YGNodeGetChildCount(node);
        for (uint32_t i = 0; i < childCount; ++i) {
            RenderNode(YGNodeGetChild(node, i), absX, absY);
        }
    }

    static auto HitTest(YGNodeRef node, float parentX, float parentY, float mouseX, float mouseY) -> YGNodeRef {
        if (!node) return nullptr;

        const float absX = parentX + YGNodeLayoutGetLeft(node);
        const float absY = parentY + YGNodeLayoutGetTop(node);
        const float width = YGNodeLayoutGetWidth(node);
        const float height = YGNodeLayoutGetHeight(node);

        const SDL_FPoint mousePoint{mouseX, mouseY};
        const SDL_FRect nodeRect{absX, absY, width, height};

        if (!SDL_PointInRectFloat(&mousePoint, &nodeRect)) return nullptr;

        const int32_t childCount = static_cast<int32_t>(YGNodeGetChildCount(node));
        for (int32_t i = childCount - 1; i >= 0; --i) {
            if (const auto hit = HitTest(YGNodeGetChild(node, i), absX, absY, mouseX, mouseY)) return hit;
        }

        if (const auto *widget = static_cast<Widget *>(YGNodeGetContext(node));
            widget && widget->type == Widget::Type::Container && widget->interactive) {
            return node;
        }
        return nullptr;
    }

    void UpdateHoverState(YGNodeRef node, YGNodeRef hoveredNode) {
        if (!node) return;

        if (auto *widget = static_cast<Widget *>(YGNodeGetContext(node))) {
            widget->hovered = (node == hoveredNode);
        }

        const uint32_t childCount = YGNodeGetChildCount(node);
        for (uint32_t i = 0; i < childCount; ++i) {
            UpdateHoverState(YGNodeGetChild(node, i), hoveredNode);
        }
    }

    ~App() {
        YGNodeFreeRecursive(root_);
        if (config_) YGConfigFree(config_);
        if (font_) TTF_CloseFont(font_);
    }

    auto Event(const SDL_Event *event) -> SDL_AppResult {
        switch (event->type) {
            case SDL_EVENT_MOUSE_MOTION: {
                const auto hitNode = HitTest(root_, 0.0f, 0.0f, event->motion.x, event->motion.y);
                UpdateHoverState(root_, hitNode);
                return SDL_APP_CONTINUE;
            }
            case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                if (event->button.button == SDL_BUTTON_LEFT) {
                    if (const auto hitNode = HitTest(root_, 0.0f, 0.0f, event->button.x, event->button.y)) {
                        if (auto *widget = static_cast<Widget *>(YGNodeGetContext(hitNode));
                            widget && widget->onClick) {
                            widget->onClick();
                        }
                    }
                }
                return SDL_APP_CONTINUE;
            }
            case SDL_EVENT_QUIT:
                return SDL_APP_SUCCESS;
            case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED: {
                const auto pixelsInPoint = chk(SDL_GetWindowDisplayScale(ctx_.window()));
                YGConfigSetPointScaleFactor(config_, pixelsInPoint);
                return SDL_APP_CONTINUE;
            }
            case SDL_EVENT_WINDOW_RESIZED:
                ctx_.SyncDimensions();
                StyleBuilder(root_)
                        .width(ctx_.width())
                        .height(ctx_.height());
                return SDL_APP_CONTINUE;
            default:
                return SDL_APP_CONTINUE;
        }
    }

    auto Iterate() -> SDL_AppResult {
        YGNodeCalculateLayout(root_, ctx_.width(), ctx_.height(), YGDirectionLTR);
        ctx_.Clear({0.05f, 0.05f, 0.06f, 1.0f});
        RenderNode(root_, 0, 0);
        ctx_.Present();
        return SDL_APP_CONTINUE;
    }

private:
    SDLLogRedirector logger_redirector_;
    WindowContext ctx_;

    TTF_Font *font_;
    YGConfigRef config_;
    YGNodeRef root_;

    std::array<std::byte, 16384> stack_buffer_;
    std::pmr::monotonic_buffer_resource upstream_arena_{stack_buffer_.data(), stack_buffer_.size()};
    std::pmr::unsynchronized_pool_resource slab_pool_{&upstream_arena_};
    std::pmr::polymorphic_allocator<Widget> widget_allocator_{&slab_pool_};
};

#include <codotaku_entry.hpp>
