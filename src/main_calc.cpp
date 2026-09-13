#include <windows.h>
#include "calc_gui/calc_window.h"

int main() {
    calc_gui::CalcWindow window;
    if (!window.Init(GetModuleHandleW(nullptr), L"CAGIO CG 50")) {
        return 1;
    }
    return window.RunMessageLoop();
}
