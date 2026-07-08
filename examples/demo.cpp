#include <array>
#include <codotaku_utils.hpp>
#include <memory_resource>
#include <span>

#include "SDL3/SDL_render.h"
#include "SDL3_ttf/SDL_ttf.h"
#include "yoga/YGConfig.h"
#include "yoga/YGNode.h"
#include "yoga/YGNodeLayout.h"
#include "yoga/YGNodeStyle.h"
#include "yoga/YGValue.h"

struct Widget {
    SDL_FColor color;
    TTF_Text *text;
};

static YGSize MeasureTextNode(YGNodeConstRef node, float width, YGMeasureMode widthMode, float height,
                              YGMeasureMode heightMode) {
    if (const auto *widget = static_cast<const Widget *>(YGNodeGetContext(node)); widget && widget->text) {
        // If Yoga tells us a target maximum layout width, pass it down to wrap the text
        if (widthMode != YGMeasureModeUndefined) {
            chk(TTF_SetTextWrapWidth(widget->text, static_cast<int>(width)));
        } else {
            chk(TTF_SetTextWrapWidth(widget->text, 0)); // No wrapping
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
    explicit App(std::span<char *> args) {
        chk(SDL_SetAppMetadata("Codotaku Demo", "1.0", "com.codotaku.demo"));
        chk(SDL_Init(SDL_INIT_VIDEO));
        chk(TTF_Init());

        chk(SDL_CreateWindowAndRenderer("Codotaku Demo", 800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN, &window_,
                                        &renderer_));
        chk(SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND));
        textEngine_ = TTF_CreateRendererTextEngine(renderer_);

        chk(SDL_GetWindowSize(window_, &windowWidth, &windowHeight));
        const auto pixelsInPoint = chk(SDL_GetWindowDisplayScale(window_));

        auto base_path = SDL_GetBasePath();
        auto font_path = std::format("{}{}", base_path, "assets/fonts/Inter_18pt-Regular.ttf");
        font_ = chk(TTF_OpenFont(font_path.c_str(), 16));

        config_ = YGConfigNew();
        YGConfigSetPointScaleFactor(config_, pixelsInPoint);

        // 1. Create the Root Node (fills the viewport canvas background)
        root_ = CreateWidget(nullptr, Widget{.color = {0.1f, 0.1f, 0.12f, 1.0f}}, [this](YGNodeRef node) {
            YGNodeStyleSetFlexDirection(node, YGFlexDirectionRow);
            YGNodeStyleSetWidth(node, windowWidth);
            YGNodeStyleSetHeight(node, windowHeight);
            YGNodeStyleSetPadding(node, YGEdgeAll, 16.0f);
        });

        // 2. Create a Sidebar Node
        auto sidebar = CreateWidget(root_, Widget{.color = {0.15f, 0.15f, 0.18f, 1.0f}}, [](YGNodeRef node) {
            YGNodeStyleSetWidth(node, 240.0f);
            YGNodeStyleSetHeightPercent(node, 100.0f);
            YGNodeStyleSetMargin(node, YGEdgeRight, 16.0f);
            YGNodeStyleSetPadding(node, YGEdgeAll, 12.0f);
            YGNodeStyleSetFlexDirection(node, YGFlexDirectionColumn);
        });

        // Sidebar Header Title (Transparent background)
        CreateTextWidget(sidebar, "DASHBOARD", SDL_FColor{0.0f, 0.0f, 0.0f, 0.0f}, [](YGNodeRef node) {
            YGNodeStyleSetMargin(node, YGEdgeBottom, 24.0f);
            YGNodeStyleSetMargin(node, YGEdgeLeft, 8.0f);
        });

        // Interactive Sidebar Navigation Links
        const std::array<const char *, 3> navItems = {"Overview", "Analytics", "Settings"};
        for (const auto *item: navItems) {
            CreateTextWidget(sidebar, item, SDL_FColor{0.2f, 0.22f, 0.28f, 1.0f}, [](YGNodeRef node) {
                YGNodeStyleSetHeight(node, 40.0f);
                YGNodeStyleSetWidthPercent(node, 100.0f);
                YGNodeStyleSetMargin(node, YGEdgeBottom, 8.0f);
                YGNodeStyleSetPadding(node, YGEdgeLeft, 12.0f);
                YGNodeStyleSetAlignItems(node, YGAlignCenter);
                YGNodeStyleSetJustifyContent(node, YGJustifyCenter);
            });
        }

        // 3. Create Main Content Area (Vertical Stack)
        auto mainContent = CreateWidget(root_, Widget{.color = {0.12f, 0.12f, 0.15f, 1.0f}}, [](YGNodeRef node) {
            YGNodeStyleSetFlexGrow(node, 1.0f);
            YGNodeStyleSetHeightPercent(node, 100.0f);
            YGNodeStyleSetPadding(node, YGEdgeAll, 24.0f);
            YGNodeStyleSetFlexDirection(node, YGFlexDirectionColumn);
        });

        // Main View Title Heading
        CreateTextWidget(mainContent, "System Settings", SDL_FColor{0.0f, 0.0f, 0.0f, 0.0f}, [](YGNodeRef node) {
            YGNodeStyleSetMargin(node, YGEdgeBottom, 16.0f);
        });

        // Content Card demonstrating Multi-line Auto-Wrapping text managed by Yoga
        auto card = CreateWidget(mainContent, Widget{.color = {0.18f, 0.18f, 0.22f, 1.0f}}, [](YGNodeRef node) {
            YGNodeStyleSetWidthPercent(node, 100.0f);
            YGNodeStyleSetPadding(node, YGEdgeAll, 16.0f);
            YGNodeStyleSetMargin(node, YGEdgeBottom, 20.0f);
            YGNodeStyleSetFlexDirection(node, YGFlexDirectionColumn);
        });

        const char *longParagraph =
                "Yoga is dynamically calculating layout bounds, passing maximum widths to our text widget, "
                "and forcing lines to auto-wrap into clean paragraph boxes when boundaries push up against the canvas container edge. "
                "Click any structural layout block to toggle its color asset context instantly.";

        CreateTextWidget(card, longParagraph, SDL_FColor{0.0f, 0.0f, 0.0f, 0.0f}, [](YGNodeRef node) {
            YGNodeStyleSetWidthPercent(node, 100.0f);
        });

        // Action Footer Area containing layout control buttons side-by-side
        auto footer = CreateWidget(mainContent, Widget{.color = {0.0f, 0.0f, 0.0f, 0.0f}}, [](YGNodeRef node) {
            YGNodeStyleSetFlexDirection(node, YGFlexDirectionRow);
            YGNodeStyleSetWidthPercent(node, 100.0f);
        });

        CreateTextWidget(footer, "Save Changes", SDL_FColor{0.25f, 0.45f, 0.35f, 1.0f}, [](YGNodeRef node) {
            YGNodeStyleSetPadding(node, YGEdgeHorizontal, 20.0f);
            YGNodeStyleSetHeight(node, 40.0f);
            YGNodeStyleSetMargin(node, YGEdgeRight, 12.0f);
        });

        CreateTextWidget(footer, "Cancel", SDL_FColor{0.35f, 0.2f, 0.25f, 1.0f}, [](YGNodeRef node) {
            YGNodeStyleSetPadding(node, YGEdgeHorizontal, 20.0f);
            YGNodeStyleSetHeight(node, 40.0f);
        });

        chk(SDL_ShowWindow(window_));
    }

    auto CreateWidget(YGNodeRef parent, Widget widget, auto &&styling_callback) -> YGNodeRef {
        auto node = YGNodeNewWithConfig(config_);

        Widget *allocated_widget = widget_allocator_.new_object<Widget>(std::move(widget));
        YGNodeSetContext(node, allocated_widget);

        if (parent) {
            YGNodeInsertChild(parent, node, YGNodeGetChildCount(parent));
        }

        styling_callback(node);
        return node;
    }

    auto CreateTextWidget(YGNodeRef parent, const char *content, SDL_FColor color,
                          auto &&styling_callback) -> YGNodeRef {
        TTF_Text *textObj = chk(TTF_CreateText(textEngine_, font_, content, 0));

        auto node = CreateWidget(parent, Widget{.color = color, .text = textObj},
                                 std::forward<decltype(styling_callback)>(styling_callback));

        // Wire up Yoga measuring mechanics so it knows how big this text block is
        YGNodeSetMeasureFunc(node, MeasureTextNode);
        return node;
    }

    void RenderNode(YGNodeRef node, float parentX, float parentY) {
        if (!node) return;

        // Calculate absolute coordinates inside the window viewport
        float absX = parentX + YGNodeLayoutGetLeft(node);
        float absY = parentY + YGNodeLayoutGetTop(node);
        float width = YGNodeLayoutGetWidth(node);
        float height = YGNodeLayoutGetHeight(node);

        auto *widget = static_cast<Widget *>(YGNodeGetContext(node));
        if (widget) {
            SDL_FRect rect{absX, absY, width, height};
            chk(SDL_SetRenderDrawColorFloat(renderer_, widget->color.r, widget->color.g, widget->color.b,
                                            widget->color.a));
            chk(SDL_RenderFillRect(renderer_, &rect));

            if (widget->text) {
                chk(TTF_SetTextColorFloat(widget->text, 1.0f, 1.0f, 1.0f, 1.0f)); // White text
                chk(TTF_SetTextPosition(widget->text, static_cast<int>(absX), static_cast<int>(absY)));
                chk(TTF_DrawRendererText(widget->text, 0, 0)); // Draws using absolute position internal state
            }
        }

        const uint32_t childCount = YGNodeGetChildCount(node);
        for (uint32_t i = 0; i < childCount; ++i) {
            RenderNode(YGNodeGetChild(node, i), absX, absY);
        }
    }

    auto HitTest(YGNodeRef node, float parentX, float parentY, float mouseX, float mouseY) -> YGNodeRef {
        if (!node) return nullptr;

        // Calculate absolute bounds for this node
        float absX = parentX + YGNodeLayoutGetLeft(node);
        float absY = parentY + YGNodeLayoutGetTop(node);
        float width = YGNodeLayoutGetWidth(node);
        float height = YGNodeLayoutGetHeight(node);

        // Check if mouse is inside this node's bounding box
        bool inside = (mouseX >= absX && mouseX <= absX + width &&
                       mouseY >= absY && mouseY <= absY + height);

        if (!inside) return nullptr;

        // Search children in reverse order (top-most visual elements first)
        int32_t childCount = static_cast<int32_t>(YGNodeGetChildCount(node));
        for (int32_t i = childCount - 1; i >= 0; --i) {
            auto hit = HitTest(YGNodeGetChild(node, i), absX, absY, mouseX, mouseY);
            if (hit) return hit; // Return the deeply nested child if hit
        }

        // If inside this node but no child was hit, this node is the target
        return node;
    }

    ~App() {
        YGNodeFreeRecursive(root_);
        if (font_) TTF_CloseFont(font_);
        if (textEngine_) TTF_DestroyRendererTextEngine(textEngine_);
        TTF_Quit();
    }

    auto Event(SDL_Event *event) -> SDL_AppResult {
        switch (event->type) {
            case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                if (event->button.button == SDL_BUTTON_LEFT) {
                    auto hitNode = HitTest(root_, 0.0f, 0.0f, event->button.x, event->button.y);
                    if (hitNode) {
                        auto *widget = static_cast<Widget *>(YGNodeGetContext(hitNode));
                        if (widget) {
                            // Quick test: invert color on click to prove it works
                            widget->color.r = 1.0f - widget->color.r;
                            widget->color.g = 1.0f - widget->color.g;
                            widget->color.b = 1.0f - widget->color.b;
                        }
                    }
                }
                return SDL_APP_CONTINUE;
            }
            case SDL_EVENT_QUIT:
                return SDL_APP_SUCCESS;
            case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED: {
                const auto pixelsInPoint = chk(SDL_GetWindowDisplayScale(window_));
                YGConfigSetPointScaleFactor(config_, pixelsInPoint);
            }
            case SDL_EVENT_WINDOW_RESIZED:
                chk(SDL_GetWindowSize(window_, &windowWidth, &windowHeight));
                YGNodeStyleSetWidth(root_, windowWidth);
                YGNodeStyleSetHeight(root_, windowHeight);
                return SDL_APP_CONTINUE;
            default:
                return SDL_APP_CONTINUE;
        }
    }

    auto Iterate() -> SDL_AppResult {
        YGNodeCalculateLayout(root_, windowWidth, windowHeight, YGDirectionLTR);
        chk(SDL_SetRenderDrawColorFloat(renderer_, 1.0, 0.0, 0.0, 1.0));
        chk(SDL_RenderClear(renderer_));
        RenderNode(root_, 0, 0);
        chk(SDL_RenderPresent(renderer_));
        return SDL_APP_CONTINUE;
    }

private
:
    SDLLogRedirector logger_redirector_;
    SDL_Window *window_;
    SDL_Renderer *renderer_;
    TTF_TextEngine *textEngine_;

    TTF_Font *font_;

    int windowWidth, windowHeight;

    YGConfigRef config_;
    YGNodeRef root_;

    std::array<std::byte, 16384> stack_buffer_;
    std::pmr::monotonic_buffer_resource upstream_arena_{stack_buffer_.data(), stack_buffer_.size()};
    std::pmr::unsynchronized_pool_resource slab_pool_{&upstream_arena_};
    std::pmr::polymorphic_allocator<Widget> widget_allocator_{&slab_pool_};
};

#include <codotaku_entry.hpp>
