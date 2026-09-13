#pragma once
#include <SDL.h>
#include <string>
#include "../mem/bus.h"

namespace gui {

// SDL2 front-end: renders the LCD framebuffer scaled up into a window and
// forwards PC keyboard events into the emulated key matrix.
class Window {
public:
    Window(int scale = 2);
    ~Window();

    bool Init(const std::string& title);
    void Shutdown();

    // Pumps SDL events; returns false when the user asked to quit.
    bool PollEvents(emu::Bus& bus);

    void Present(emu::Bus& bus);

private:
    int scale_;
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* texture_ = nullptr;

    // Maps an SDL scancode to a (row, col) in the emulated keypad, or
    // returns false if the key isn't mapped.
    bool MapKey(SDL_Scancode sc, int& row, int& col) const;
};

} // namespace gui
