#include "window.h"

namespace gui {

namespace {
constexpr wchar_t kClassName[] = L"CG50EmuWindowClass";

// Simplified key layout (row, col) in the 8x8 matrix. This is an original
// mapping (not a reproduction of Casio's real scan matrix), chosen to be
// convenient with a PC keyboard. See docs/HARDWARE.md.
struct KeyMap { int vk; int row; int col; };
const KeyMap kKeyMap[] = {
    { VK_F1, 0, 0 }, { VK_F2, 0, 1 }, { VK_F3, 0, 2 },
    { VK_F4, 0, 3 }, { VK_F5, 0, 4 }, { VK_F6, 0, 5 },
    { VK_SHIFT, 0, 6 }, { VK_MENU, 0, 7 }, // SHIFT / ALPHA

    { VK_UP, 1, 0 }, { VK_DOWN, 1, 1 },
    { VK_LEFT, 1, 2 }, { VK_RIGHT, 1, 3 },
    { VK_ESCAPE, 1, 4 }, { VK_TAB, 1, 5 }, // EXIT / MENU

    { '7', 2, 0 }, { '8', 2, 1 }, { '9', 2, 2 }, { VK_DIVIDE, 2, 3 },
    { '4', 3, 0 }, { '5', 3, 1 }, { '6', 3, 2 }, { VK_MULTIPLY, 3, 3 },
    { '1', 4, 0 }, { '2', 4, 1 }, { '3', 4, 2 }, { VK_SUBTRACT, 4, 3 },
    { '0', 5, 0 }, { VK_OEM_PERIOD, 5, 1 }, { VK_RETURN, 5, 2 }, { VK_ADD, 5, 3 },
};
constexpr int kKeyMapCount = sizeof(kKeyMap) / sizeof(kKeyMap[0]);
} // namespace

Window::Window(int scale) : scale_(scale) {}
Window::~Window() { Shutdown(); }

bool Window::MapKey(WPARAM vk, int& row, int& col) {
    for (int i = 0; i < kKeyMapCount; ++i) {
        if (kKeyMap[i].vk == static_cast<int>(vk)) { row = kKeyMap[i].row; col = kKeyMap[i].col; return true; }
    }
    return false;
}

LRESULT CALLBACK Window::WndProcThunk(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    Window* self;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = reinterpret_cast<Window*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    if (self) return self->HandleMessage(hwnd, msg, wp, lp);
    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT Window::HandleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CLOSE:
    case WM_DESTROY:
        quit_ = true;
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
    case WM_KEYUP: {
        int row, col;
        if (bus_ && MapKey(wp, row, col)) {
            bus_->Keypad().SetKey(row, col, msg == WM_KEYDOWN);
        }
        return 0;
    }
    default:
        return DefWindowProcW(hwnd, msg, wp, lp);
    }
}

bool Window::Init(const std::string& title) {
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &Window::WndProcThunk;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = kClassName;
    if (!RegisterClassExW(&wc)) return false;

    RECT rect = { 0, 0, devices::Lcd::kWidth * scale_, devices::Lcd::kHeight * scale_ };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    std::wstring wtitle(title.begin(), title.end());
    hwnd_ = CreateWindowExW(0, kClassName, wtitle.c_str(), WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             rect.right - rect.left, rect.bottom - rect.top,
                             nullptr, nullptr, wc.hInstance, this);
    if (!hwnd_) return false;

    ShowWindow(hwnd_, SW_SHOWDEFAULT);
    UpdateWindow(hwnd_);
    return true;
}

void Window::Shutdown() {
    if (hwnd_) { DestroyWindow(hwnd_); hwnd_ = nullptr; }
}

bool Window::PollEvents(emu::Bus& bus) {
    bus_ = &bus;
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) quit_ = true;
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return !quit_;
}

void Window::Present(emu::Bus& bus) {
    if (!hwnd_) return;
    const auto& fb = bus.Lcd().Framebuffer();

    struct { BITMAPINFOHEADER header; DWORD masks[3]; } bmi = {};
    bmi.header.biSize = sizeof(BITMAPINFOHEADER);
    bmi.header.biWidth = devices::Lcd::kWidth;
    bmi.header.biHeight = -devices::Lcd::kHeight; // negative = top-down source
    bmi.header.biPlanes = 1;
    bmi.header.biBitCount = 16;
    bmi.header.biCompression = BI_BITFIELDS;
    bmi.masks[0] = 0xF800; // R
    bmi.masks[1] = 0x07E0; // G
    bmi.masks[2] = 0x001F; // B

    HDC hdc = GetDC(hwnd_);
    RECT client;
    GetClientRect(hwnd_, &client);
    StretchDIBits(hdc, 0, 0, client.right - client.left, client.bottom - client.top,
                  0, 0, devices::Lcd::kWidth, devices::Lcd::kHeight,
                  fb.data(), reinterpret_cast<BITMAPINFO*>(&bmi), DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(hwnd_, hdc);
}

} // namespace gui
