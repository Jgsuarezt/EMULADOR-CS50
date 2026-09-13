#pragma once
#include <windows.h>
#include <string>
#include "../mem/bus.h"

namespace gui {

// Native Win32 front-end (GDI): renders the LCD framebuffer scaled up into
// a plain window and forwards keyboard events into the emulated key
// matrix. No external dependency (no SDL2) -- only the Windows API, which
// is always present.
class Window {
public:
    explicit Window(int scale = 2);
    ~Window();

    bool Init(const std::string& title);
    void Shutdown();

    // Pumps the Win32 message queue; returns false when the user closed
    // the window.
    bool PollEvents(emu::Bus& bus);

    void Present(emu::Bus& bus);

private:
    int scale_;
    HWND hwnd_ = nullptr;
    bool quit_ = false;
    emu::Bus* bus_ = nullptr;

    static LRESULT CALLBACK WndProcThunk(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    // Maps a Windows virtual-key code to a (row, col) in the emulated
    // keypad, or returns false if the key isn't mapped.
    static bool MapKey(WPARAM vk, int& row, int& col);
};

} // namespace gui
