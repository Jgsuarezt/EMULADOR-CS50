#include "window.h"
#include <cstdio>
#include <cstring>

namespace gui {

namespace {
// Simplified key layout (row, col) in the 8x8 matrix. This is an original
// mapping (not a reproduction of Casio's real scan matrix) chosen to be
// convenient with a PC keyboard. See docs/HARDWARE.md.
struct KeyMap { SDL_Scancode sc; int row; int col; };
const KeyMap kKeyMap[] = {
    { SDL_SCANCODE_F1, 0, 0 }, { SDL_SCANCODE_F2, 0, 1 }, { SDL_SCANCODE_F3, 0, 2 },
    { SDL_SCANCODE_F4, 0, 3 }, { SDL_SCANCODE_F5, 0, 4 }, { SDL_SCANCODE_F6, 0, 5 },
    { SDL_SCANCODE_LSHIFT, 0, 6 }, { SDL_SCANCODE_LALT, 0, 7 }, // SHIFT / ALPHA

    { SDL_SCANCODE_UP, 1, 0 }, { SDL_SCANCODE_DOWN, 1, 1 },
    { SDL_SCANCODE_LEFT, 1, 2 }, { SDL_SCANCODE_RIGHT, 1, 3 },
    { SDL_SCANCODE_ESCAPE, 1, 4 }, { SDL_SCANCODE_TAB, 1, 5 }, // EXIT / MENU

    { SDL_SCANCODE_7, 2, 0 }, { SDL_SCANCODE_8, 2, 1 }, { SDL_SCANCODE_9, 2, 2 },
    { SDL_SCANCODE_SLASH, 2, 3 },

    { SDL_SCANCODE_4, 3, 0 }, { SDL_SCANCODE_5, 3, 1 }, { SDL_SCANCODE_6, 3, 2 },
    { SDL_SCANCODE_KP_MULTIPLY, 3, 3 },

    { SDL_SCANCODE_1, 4, 0 }, { SDL_SCANCODE_2, 4, 1 }, { SDL_SCANCODE_3, 4, 2 },
    { SDL_SCANCODE_MINUS, 4, 3 },

    { SDL_SCANCODE_0, 5, 0 }, { SDL_SCANCODE_PERIOD, 5, 1 }, { SDL_SCANCODE_RETURN, 5, 2 },
    { SDL_SCANCODE_EQUALS, 5, 3 }, // EXE / ADD
};
constexpr int kKeyMapCount = sizeof(kKeyMap) / sizeof(kKeyMap[0]);
} // namespace

Window::Window(int scale) : scale_(scale) {}

Window::~Window() { Shutdown(); }

bool Window::Init(const std::string& title) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }
    window_ = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                devices::Lcd::kWidth * scale_, devices::Lcd::kHeight * scale_, 0);
    if (!window_) return false;
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer_) renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer_) return false;
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING,
                                  devices::Lcd::kWidth, devices::Lcd::kHeight);
    return texture_ != nullptr;
}

void Window::Shutdown() {
    if (texture_) { SDL_DestroyTexture(texture_); texture_ = nullptr; }
    if (renderer_) { SDL_DestroyRenderer(renderer_); renderer_ = nullptr; }
    if (window_) { SDL_DestroyWindow(window_); window_ = nullptr; }
    SDL_Quit();
}

bool Window::MapKey(SDL_Scancode sc, int& row, int& col) const {
    for (int i = 0; i < kKeyMapCount; ++i) {
        if (kKeyMap[i].sc == sc) { row = kKeyMap[i].row; col = kKeyMap[i].col; return true; }
    }
    return false;
}

bool Window::PollEvents(emu::Bus& bus) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) return false;
        if (ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP) {
            int row, col;
            if (MapKey(ev.key.keysym.scancode, row, col)) {
                bus.Keypad().SetKey(row, col, ev.type == SDL_KEYDOWN);
            }
            if (ev.key.keysym.scancode == SDL_SCANCODE_Q && (ev.key.keysym.mod & KMOD_CTRL)) {
                return false;
            }
        }
    }
    return true;
}

void Window::Present(emu::Bus& bus) {
    void* pixels = nullptr;
    int pitch = 0;
    if (SDL_LockTexture(texture_, nullptr, &pixels, &pitch) == 0) {
        const auto& fb = bus.Lcd().Framebuffer();
        auto* dst = static_cast<uint8_t*>(pixels);
        int row_bytes = devices::Lcd::kWidth * 2;
        for (int y = 0; y < devices::Lcd::kHeight; ++y) {
            std::memcpy(dst + y * pitch, fb.data() + y * row_bytes, row_bytes);
        }
        SDL_UnlockTexture(texture_);
    }
    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);
}

} // namespace gui
