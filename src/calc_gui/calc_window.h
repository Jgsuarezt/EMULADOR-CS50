#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include "../calc/engine.h"

namespace calc_gui {

enum class Action {
    None,
    AllClear,
    DeleteChar,
    Equals,
    ShiftToggle,
    ToggleAngleMode,
    HistoryUp,
    HistoryDown,
    CursorLeft,
    CursorRight,
};

struct Button {
    RECT rect{};
    std::wstring label;
    std::wstring shift_label;       // small label, shown top-left in yellow
    COLORREF bg = RGB(255, 255, 255);
    COLORREF fg = RGB(0, 0, 0);
    std::wstring insert_text;       // inserted at the cursor on a plain tap
    std::wstring shift_insert_text; // inserted instead when SHIFT is active
    bool auto_close_paren = false;  // also insert a matching ')' and place the cursor between them
    Action special = Action::None;
    Action shift_special = Action::None;
};

// A visual + functional recreation (original artwork, not traced from any
// Casio asset) of a scientific graphing-calculator keypad, wired to a real
// expression engine (src/calc/engine.h). Branded "CAGIO CG 50" rather than
// the real product name.
class CalcWindow {
public:
    bool Init(HINSTANCE hinst, const std::wstring& title);
    int RunMessageLoop();

private:
    HWND hwnd_ = nullptr;
    calc::Engine engine_;

    std::wstring input_;
    size_t cursor_pos_ = 0;
    std::wstring result_;
    std::wstring error_;
    bool just_evaluated_ = false;
    bool shift_active_ = false;
    bool cursor_visible_ = true;

    std::vector<std::wstring> history_;
    int history_index_ = -1;

    std::vector<Button> buttons_;
    RECT screen_rect_{};

    static constexpr int kWidth = 400;
    static constexpr int kHeight = 850;

    void BuildLayout();
    void AddRow(int y, int h, std::vector<Button> specs, int margin, int content_w);

    void Paint(HDC hdc);
    void DrawScreen(HDC hdc);
    void DrawButton(HDC hdc, const Button& b);

    void OnLButtonDown(int x, int y);
    void HandleButtonPress(const Button& b);
    void InsertText(const std::wstring& s, bool auto_close);
    void DoDelete();
    void DoEquals();
    void DoAllClear();
    void DoHistory(int direction);
    void MoveCursor(int delta);

    void OnChar(wchar_t c);
    void OnKeyDown(WPARAM vk);

    static LRESULT CALLBACK WndProcThunk(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
};

} // namespace calc_gui
