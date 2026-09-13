#include "calc_window.h"
#include <windowsx.h>
#include <cmath>
#include <cstdio>
#include <cwchar>

namespace calc_gui {

namespace {
constexpr wchar_t kClassName[] = L"CG50CalcWindowClass";
constexpr UINT_PTR kBlinkTimerId = 1;

COLORREF Body() { return RGB(0xEC, 0xEC, 0xE7); }
COLORREF Bezel() { return RGB(0x22, 0x2B, 0x3A); }
COLORREF ScreenBg() { return RGB(0xF4, 0xF7, 0xEE); }
COLORREF TextDark() { return RGB(0x14, 0x18, 0x1C); }
COLORREF ErrorRed() { return RGB(0xA3, 0x1E, 0x1E); }

COLORREF FKeyBg() { return RGB(0x46, 0x46, 0x46); }
COLORREF ShiftBg() { return RGB(0xF3, 0xC5, 0x18); }
COLORREF AlphaLikeBg() { return RGB(0xD9, 0xD9, 0xD6); }
COLORREF MenuBg() { return RGB(0x2B, 0x3E, 0x74); }
COLORREF ExitBg() { return RGB(0x82, 0x22, 0x22); }
COLORREF ArrowBg() { return RGB(0x1C, 0x28, 0x40); }
COLORREF SciBg() { return RGB(0xDC, 0xE6, 0xF0); }
COLORREF NumBg() { return RGB(0xFF, 0xFF, 0xFF); }
COLORREF OpBg() { return RGB(0x3A, 0x3A, 0x3A); }
COLORREF AcBg() { return RGB(0xB0, 0x35, 0x35); }
COLORREF ExeBg() { return RGB(0x2E, 0x7D, 0x32); }

Button MakeBtn(const wchar_t* label, COLORREF bg, COLORREF fg,
               const wchar_t* insert = L"", bool auto_close = false,
               Action special = Action::None,
               const wchar_t* shift_label = L"", const wchar_t* shift_insert = L"",
               Action shift_special = Action::None) {
    Button b;
    b.label = label;
    b.bg = bg;
    b.fg = fg;
    b.insert_text = insert;
    b.auto_close_paren = auto_close;
    b.special = special;
    b.shift_label = shift_label;
    b.shift_insert_text = shift_insert;
    b.shift_special = shift_special;
    return b;
}
} // namespace

void CalcWindow::AddRow(int y, int h, std::vector<Button> specs, int margin, int content_w) {
    int n = static_cast<int>(specs.size());
    if (n == 0) return;
    int gap = 6;
    int bw = (content_w - gap * (n - 1)) / n;
    int x = margin;
    for (auto& b : specs) {
        b.rect = { x, y, x + bw, y + h };
        buttons_.push_back(b);
        x += bw + gap;
    }
}

void CalcWindow::BuildLayout() {
    buttons_.clear();
    const int margin = 14;
    const int content_w = kWidth - 2 * margin;
    const int row_h = 44;
    const int gap = 6;

    // Brand label + screen.
    screen_rect_ = { margin, 46, kWidth - margin, 46 + 190 };
    int y = screen_rect_.bottom + 18;

    // F1..F6 (soft keys, decorative -- there is no menu system behind them yet).
    {
        std::vector<Button> row;
        for (int i = 1; i <= 6; ++i) {
            wchar_t buf[4];
            swprintf(buf, 4, L"F%d", i);
            row.push_back(MakeBtn(buf, FKeyBg(), RGB(255, 255, 255)));
        }
        AddRow(y, 34, row, margin, content_w);
        y += 34 + gap;
    }

    // SHIFT / OPTN / VARS / MENU.
    {
        std::vector<Button> row;
        row.push_back(MakeBtn(L"SHIFT", ShiftBg(), RGB(0, 0, 0), L"", false, Action::ShiftToggle));
        row.push_back(MakeBtn(L"OPTN", AlphaLikeBg(), RGB(0, 0, 0)));
        row.push_back(MakeBtn(L"VARS", AlphaLikeBg(), RGB(0, 0, 0)));
        row.push_back(MakeBtn(L"MENU", MenuBg(), RGB(255, 255, 255), L"", false, Action::ToggleAngleMode));
        AddRow(y, row_h, row, margin, content_w);
        y += row_h + gap;
    }

    // EXIT + arrow pad (cursor / history) + a decorative X,theta,T-style key.
    {
        int pad_h = 118;
        int exit_w = 76;
        int deco_w = 76;
        int pad_w = content_w - exit_w - deco_w - 2 * gap;
        int pad_x = margin + exit_w + gap;

        Button exit_btn = MakeBtn(L"EXIT", ExitBg(), RGB(255, 255, 255), L"", false, Action::AllClear);
        exit_btn.rect = { margin, y + pad_h / 2 - 20, margin + exit_w, y + pad_h / 2 + 20 };
        buttons_.push_back(exit_btn);

        Button deco = MakeBtn(L"X,θ,T", AlphaLikeBg(), RGB(0, 0, 0));
        deco.rect = { margin + exit_w + gap + pad_w + gap, y + pad_h / 2 - 20,
                      margin + exit_w + gap + pad_w + gap + deco_w, y + pad_h / 2 + 20 };
        buttons_.push_back(deco);

        int cell = pad_w / 3;
        int cell_h = pad_h / 3;
        int px = pad_x + (pad_w - cell) / 2;

        Button up = MakeBtn(L"▲", ArrowBg(), RGB(255, 255, 255), L"", false, Action::HistoryUp);
        up.rect = { px, y, px + cell, y + cell_h };
        buttons_.push_back(up);

        Button down = MakeBtn(L"▼", ArrowBg(), RGB(255, 255, 255), L"", false, Action::HistoryDown);
        down.rect = { px, y + 2 * cell_h, px + cell, y + pad_h };
        buttons_.push_back(down);

        Button left = MakeBtn(L"<", ArrowBg(), RGB(255, 255, 255), L"", false, Action::CursorLeft);
        left.rect = { pad_x, y + cell_h, pad_x + cell, y + 2 * cell_h };
        buttons_.push_back(left);

        Button right = MakeBtn(L">", ArrowBg(), RGB(255, 255, 255), L"", false, Action::CursorRight);
        right.rect = { pad_x + pad_w - cell, y + cell_h, pad_x + pad_w, y + 2 * cell_h };
        buttons_.push_back(right);

        y += pad_h + gap;
    }

    // Scientific functions, row 1.
    {
        std::vector<Button> row;
        row.push_back(MakeBtn(L"x²", SciBg(), RGB(0, 0, 0), L"^2"));
        row.push_back(MakeBtn(L"^", SciBg(), RGB(0, 0, 0), L"^"));
        row.push_back(MakeBtn(L"10^x", SciBg(), RGB(0, 0, 0), L"10^(", true));
        row.push_back(MakeBtn(L"log", SciBg(), RGB(0, 0, 0), L"log(", true));
        row.push_back(MakeBtn(L"ln", SciBg(), RGB(0, 0, 0), L"ln(", true));
        row.push_back(MakeBtn(L"(-)", SciBg(), RGB(0, 0, 0), L"-"));
        AddRow(y, row_h, row, margin, content_w);
        y += row_h + gap;
    }

    // Scientific functions, row 2 (with SHIFT = inverse trig).
    {
        std::vector<Button> row;
        row.push_back(MakeBtn(L"sin", SciBg(), RGB(0, 0, 0), L"sin(", true, Action::None,
                               L"sin⁻¹", L"asin(", Action::None));
        row.push_back(MakeBtn(L"cos", SciBg(), RGB(0, 0, 0), L"cos(", true, Action::None,
                               L"cos⁻¹", L"acos(", Action::None));
        row.push_back(MakeBtn(L"tan", SciBg(), RGB(0, 0, 0), L"tan(", true, Action::None,
                               L"tan⁻¹", L"atan(", Action::None));
        row.push_back(MakeBtn(L"√", SciBg(), RGB(0, 0, 0), L"sqrt(", true));
        row.push_back(MakeBtn(L"(", SciBg(), RGB(0, 0, 0), L"("));
        row.push_back(MakeBtn(L")", SciBg(), RGB(0, 0, 0), L")"));
        AddRow(y, row_h, row, margin, content_w);
        y += row_h + gap;
    }

    // AC / DEL / Ans / %.
    {
        std::vector<Button> row;
        row.push_back(MakeBtn(L"AC", AcBg(), RGB(255, 255, 255), L"", false, Action::AllClear));
        row.push_back(MakeBtn(L"DEL", AlphaLikeBg(), RGB(0, 0, 0), L"", false, Action::DeleteChar));
        row.push_back(MakeBtn(L"Ans", SciBg(), RGB(0, 0, 0), L"Ans"));
        row.push_back(MakeBtn(L"%", SciBg(), RGB(0, 0, 0), L"%"));
        AddRow(y, row_h, row, margin, content_w);
        y += row_h + gap;
    }

    // Numeric pad + operators.
    const wchar_t* rows[4][4] = {
        { L"7", L"8", L"9", L"÷" },
        { L"4", L"5", L"6", L"×" },
        { L"1", L"2", L"3", L"−" },
        { L"0", L".", L"EXE", L"+" },
    };
    const wchar_t* inserts[4][4] = {
        { L"7", L"8", L"9", L"/" },
        { L"4", L"5", L"6", L"*" },
        { L"1", L"2", L"3", L"-" },
        { L"0", L".", L"", L"+" },
    };
    for (int r = 0; r < 4; ++r) {
        std::vector<Button> row;
        for (int c = 0; c < 4; ++c) {
            bool is_op = (c == 3);
            bool is_exe = (r == 3 && c == 2);
            COLORREF bg = is_exe ? ExeBg() : (is_op ? OpBg() : NumBg());
            COLORREF fg = (is_exe || is_op) ? RGB(255, 255, 255) : RGB(0, 0, 0);
            Action sp = is_exe ? Action::Equals : Action::None;
            row.push_back(MakeBtn(rows[r][c], bg, fg, is_exe ? L"" : inserts[r][c], false, sp));
        }
        AddRow(y, row_h, row, margin, content_w);
        y += row_h + gap;
    }
}

void CalcWindow::InsertText(const std::wstring& s, bool auto_close) {
    if (just_evaluated_) {
        just_evaluated_ = false;
        wchar_t c = s.empty() ? 0 : s[0];
        bool is_operator = (c == L'+' || c == L'-' || c == L'*' || c == L'/' || c == L'^' || c == L'%');
        input_ = is_operator ? (L"Ans" + s) : s;
        cursor_pos_ = input_.size();
        error_.clear();
        if (auto_close) {
            input_ += L")";
            cursor_pos_ = input_.size() - 1;
        }
        return;
    }
    input_.insert(cursor_pos_, s);
    cursor_pos_ += s.size();
    if (auto_close) {
        input_.insert(cursor_pos_, L")");
        // cursor stays right after the inserted '(' / function-open text
    }
    error_.clear();
}

void CalcWindow::DoDelete() {
    if (just_evaluated_) { DoAllClear(); return; }
    if (cursor_pos_ > 0 && !input_.empty()) {
        input_.erase(cursor_pos_ - 1, 1);
        --cursor_pos_;
    }
}

void CalcWindow::DoAllClear() {
    input_.clear();
    cursor_pos_ = 0;
    result_.clear();
    error_.clear();
    just_evaluated_ = false;
}

void CalcWindow::MoveCursor(int delta) {
    long np = static_cast<long>(cursor_pos_) + delta;
    if (np < 0) np = 0;
    if (static_cast<size_t>(np) > input_.size()) np = static_cast<long>(input_.size());
    cursor_pos_ = static_cast<size_t>(np);
}

void CalcWindow::DoEquals() {
    if (input_.empty()) return;
    try {
        double r = engine_.Evaluate(input_);
        wchar_t buf[64];
        if (std::fabs(r - std::round(r)) < 1e-9 && std::fabs(r) < 1e15) {
            swprintf(buf, 64, L"%.0f", r);
        } else {
            swprintf(buf, 64, L"%.10g", r);
        }
        result_ = buf;
        error_.clear();
        history_.push_back(input_);
        history_index_ = -1;
    } catch (const std::exception& e) {
        std::string msg = e.what();
        error_.assign(msg.begin(), msg.end());
        result_.clear();
    }
    just_evaluated_ = true;
}

void CalcWindow::DoHistory(int direction) {
    if (history_.empty()) return;
    if (direction < 0) { // Up: older
        if (history_index_ == -1) history_index_ = static_cast<int>(history_.size()) - 1;
        else if (history_index_ > 0) --history_index_;
    } else { // Down: newer
        if (history_index_ == -1) return;
        if (history_index_ < static_cast<int>(history_.size()) - 1) ++history_index_;
        else history_index_ = -1;
    }
    input_ = (history_index_ == -1) ? std::wstring() : history_[history_index_];
    cursor_pos_ = input_.size();
    just_evaluated_ = false;
}

void CalcWindow::HandleButtonPress(const Button& b) {
    if (b.special == Action::ShiftToggle) {
        shift_active_ = !shift_active_;
        return; // do not consume shift on the SHIFT key itself
    }

    bool use_shift = shift_active_;
    Action act = (use_shift && b.shift_special != Action::None) ? b.shift_special : b.special;
    const std::wstring& text = (use_shift && !b.shift_insert_text.empty()) ? b.shift_insert_text : b.insert_text;

    switch (act) {
    case Action::AllClear: DoAllClear(); break;
    case Action::DeleteChar: DoDelete(); break;
    case Action::Equals: DoEquals(); break;
    case Action::ToggleAngleMode:
        engine_.SetAngleMode(engine_.GetAngleMode() == calc::AngleMode::Degree ? calc::AngleMode::Radian
                                                                                : calc::AngleMode::Degree);
        break;
    case Action::HistoryUp: DoHistory(-1); break;
    case Action::HistoryDown: DoHistory(1); break;
    case Action::CursorLeft: MoveCursor(-1); break;
    case Action::CursorRight: MoveCursor(1); break;
    default:
        if (!text.empty()) InsertText(text, b.auto_close_paren);
        break;
    }
    shift_active_ = false; // one-shot, like a real scientific calculator
}

void CalcWindow::OnLButtonDown(int x, int y) {
    POINT pt{ x, y };
    for (const auto& b : buttons_) {
        if (PtInRect(&b.rect, pt)) {
            HandleButtonPress(b);
            InvalidateRect(hwnd_, nullptr, FALSE);
            return;
        }
    }
}

void CalcWindow::OnChar(wchar_t c) {
    if (c == L'\r') { DoEquals(); InvalidateRect(hwnd_, nullptr, FALSE); return; }
    if (c == 8) { DoDelete(); InvalidateRect(hwnd_, nullptr, FALSE); return; }
    if (c == 27) { DoAllClear(); InvalidateRect(hwnd_, nullptr, FALSE); return; }
    static const std::wstring allowed = L"0123456789.+-*/^()%";
    if (allowed.find(c) != std::wstring::npos) {
        InsertText(std::wstring(1, c), false);
        InvalidateRect(hwnd_, nullptr, FALSE);
    }
}

void CalcWindow::OnKeyDown(WPARAM vk) {
    if (vk == VK_LEFT) { MoveCursor(-1); InvalidateRect(hwnd_, nullptr, FALSE); }
    else if (vk == VK_RIGHT) { MoveCursor(1); InvalidateRect(hwnd_, nullptr, FALSE); }
    else if (vk == VK_UP) { DoHistory(-1); InvalidateRect(hwnd_, nullptr, FALSE); }
    else if (vk == VK_DOWN) { DoHistory(1); InvalidateRect(hwnd_, nullptr, FALSE); }
}

void CalcWindow::DrawButton(HDC hdc, const Button& b) {
    HBRUSH br = CreateSolidBrush(b.bg);
    HGDIOBJ old_br = SelectObject(hdc, br);
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(60, 60, 60));
    HGDIOBJ old_pen = SelectObject(hdc, pen);
    RoundRect(hdc, b.rect.left, b.rect.top, b.rect.right, b.rect.bottom, 10, 10);
    SelectObject(hdc, old_pen);
    SelectObject(hdc, old_br);
    DeleteObject(pen);
    DeleteObject(br);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, b.fg);
    RECT r = b.rect;
    DrawTextW(hdc, b.label.c_str(), -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    if (!b.shift_label.empty()) {
        SetTextColor(hdc, RGB(0x8A, 0x63, 0x00));
        RECT sr = { b.rect.left + 3, b.rect.top - 14, b.rect.right, b.rect.top + 2 };
        DrawTextW(hdc, b.shift_label.c_str(), -1, &sr, DT_LEFT | DT_TOP | DT_SINGLELINE);
    }
}

void CalcWindow::DrawScreen(HDC hdc) {
    HBRUSH bezel = CreateSolidBrush(Bezel());
    RECT outer = { screen_rect_.left - 8, screen_rect_.top - 8, screen_rect_.right + 8, screen_rect_.bottom + 8 };
    FillRect(hdc, &outer, bezel);
    DeleteObject(bezel);

    HBRUSH scr = CreateSolidBrush(ScreenBg());
    FillRect(hdc, &screen_rect_, scr);
    DeleteObject(scr);

    SetBkMode(hdc, TRANSPARENT);

    HFONT small_font = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                    DEFAULT_PITCH, L"Consolas");
    HFONT big_font = CreateFontW(30, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                  DEFAULT_PITCH, L"Consolas");

    HGDIOBJ old_font = SelectObject(hdc, small_font);
    SetTextColor(hdc, RGB(0x60, 0x60, 0x60));
    RECT mode_r = { screen_rect_.left + 8, screen_rect_.top + 4, screen_rect_.right - 8, screen_rect_.top + 22 };
    const wchar_t* mode = engine_.GetAngleMode() == calc::AngleMode::Degree ? L"Deg" : L"Rad";
    wchar_t status[32];
    swprintf(status, 32, L"%ls%ls", mode, shift_active_ ? L"   SHIFT" : L"");
    DrawTextW(hdc, status, -1, &mode_r, DT_LEFT | DT_TOP | DT_SINGLELINE);

    // Input line, left-aligned, with a blinking text cursor at cursor_pos_.
    int in_x = screen_rect_.left + 8;
    int in_y = screen_rect_.top + 30;
    SetTextColor(hdc, TextDark());
    RECT in_r = { in_x, in_y, screen_rect_.right - 8, in_y + 40 };
    std::wstring shown = input_.empty() ? L"0" : input_;
    DrawTextW(hdc, shown.c_str(), -1, &in_r, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX);

    if (cursor_visible_ && !just_evaluated_) {
        SIZE sz{};
        GetTextExtentPoint32W(hdc, input_.c_str(), static_cast<int>(cursor_pos_), &sz);
        int cx = in_x + sz.cx;
        HPEN cpen = CreatePen(PS_SOLID, 2, TextDark());
        HGDIOBJ old_pen = SelectObject(hdc, cpen);
        MoveToEx(hdc, cx, in_y + 2, nullptr);
        LineTo(hdc, cx, in_y + 26);
        SelectObject(hdc, old_pen);
        DeleteObject(cpen);
    }

    SelectObject(hdc, big_font);
    RECT out_r = { screen_rect_.left + 8, screen_rect_.top + 82, screen_rect_.right - 8, screen_rect_.top + 130 };
    if (!error_.empty()) {
        SetTextColor(hdc, ErrorRed());
        DrawTextW(hdc, error_.c_str(), -1, &out_r, DT_RIGHT | DT_TOP | DT_SINGLELINE);
    } else if (!result_.empty()) {
        SetTextColor(hdc, TextDark());
        std::wstring eq = L"= " + result_;
        DrawTextW(hdc, eq.c_str(), -1, &out_r, DT_RIGHT | DT_TOP | DT_SINGLELINE);
    }

    SelectObject(hdc, old_font);
    DeleteObject(small_font);
    DeleteObject(big_font);
}

void CalcWindow::Paint(HDC hdc) {
    RECT client;
    GetClientRect(hwnd_, &client);
    HBRUSH body = CreateSolidBrush(Body());
    FillRect(hdc, &client, body);
    DeleteObject(body);

    SetBkMode(hdc, TRANSPARENT);
    HFONT brand_font = CreateFontW(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                    DEFAULT_PITCH, L"Segoe UI");
    HGDIOBJ old_font = SelectObject(hdc, brand_font);
    SetTextColor(hdc, RGB(0x33, 0x33, 0x33));
    RECT brand_r = { 0, 10, kWidth, 34 };
    DrawTextW(hdc, L"CAGIO CG 50", -1, &brand_r, DT_CENTER | DT_TOP | DT_SINGLELINE);
    SelectObject(hdc, old_font);
    DeleteObject(brand_font);

    DrawScreen(hdc);

    HFONT btn_font = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                  DEFAULT_PITCH, L"Segoe UI");
    HFONT sm_font = CreateFontW(11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                 DEFAULT_PITCH, L"Segoe UI");
    old_font = SelectObject(hdc, btn_font);
    for (const auto& b : buttons_) {
        if (!b.shift_label.empty()) SelectObject(hdc, sm_font);
        DrawButton(hdc, b);
        if (!b.shift_label.empty()) SelectObject(hdc, btn_font);
    }
    SelectObject(hdc, old_font);
    DeleteObject(btn_font);
    DeleteObject(sm_font);
}

LRESULT CALLBACK CalcWindow::WndProcThunk(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    CalcWindow* self;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = reinterpret_cast<CalcWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<CalcWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    if (self) return self->HandleMessage(hwnd, msg, wp, lp);
    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT CalcWindow::HandleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        SetTimer(hwnd, kBlinkTimerId, 500, nullptr);
        return 0;
    case WM_TIMER:
        if (wp == kBlinkTimerId) {
            cursor_visible_ = !cursor_visible_;
            InvalidateRect(hwnd, &screen_rect_, FALSE);
        }
        return 0;
    case WM_LBUTTONDOWN:
        OnLButtonDown(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
        return 0;
    case WM_CHAR:
        OnChar(static_cast<wchar_t>(wp));
        return 0;
    case WM_KEYDOWN:
        OnKeyDown(wp);
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT client;
        GetClientRect(hwnd, &client);
        HDC mem_dc = CreateCompatibleDC(hdc);
        HBITMAP bmp = CreateCompatibleBitmap(hdc, client.right, client.bottom);
        HGDIOBJ old_bmp = SelectObject(mem_dc, bmp);
        Paint(mem_dc);
        BitBlt(hdc, 0, 0, client.right, client.bottom, mem_dc, 0, 0, SRCCOPY);
        SelectObject(mem_dc, old_bmp);
        DeleteObject(bmp);
        DeleteDC(mem_dc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        KillTimer(hwnd, kBlinkTimerId);
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(hwnd, msg, wp, lp);
    }
}

bool CalcWindow::Init(HINSTANCE hinst, const std::wstring& title) {
    BuildLayout();

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &CalcWindow::WndProcThunk;
    wc.hInstance = hinst;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszClassName = kClassName;
    if (!RegisterClassExW(&wc)) return false;

    RECT rect = { 0, 0, kWidth, kHeight };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, FALSE);

    hwnd_ = CreateWindowExW(0, kClassName, title.c_str(),
                             (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX),
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             rect.right - rect.left, rect.bottom - rect.top,
                             nullptr, nullptr, hinst, this);
    if (!hwnd_) return false;

    ShowWindow(hwnd_, SW_SHOWDEFAULT);
    UpdateWindow(hwnd_);
    return true;
}

int CalcWindow::RunMessageLoop() {
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}

} // namespace calc_gui
