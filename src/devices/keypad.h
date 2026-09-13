#pragma once
#include <cstdint>
#include <array>
#include <string>
#include <unordered_map>

namespace devices {

// Simplified 8-row x 8-column key matrix. The GUI front-end sets key state
// by name (e.g. "EXE", "F1", "0".."9", arrow keys); the CPU reads it back
// through a row-select / column-read register pair, similar in spirit to
// how the real calculator's keyboard controller is scanned (see
// docs/HARDWARE.md for the exact simplified protocol used here).
class Keypad {
public:
    static constexpr int kRows = 8;
    static constexpr int kCols = 8;

    Keypad() { matrix_.fill(0); }

    void SetKey(int row, int col, bool down) {
        if (row < 0 || row >= kRows || col < 0 || col >= kCols) return;
        uint8_t mask = 1u << col;
        if (down) matrix_[row] |= mask;
        else matrix_[row] &= ~mask;
    }

    void SelectRow(uint8_t row) { selected_row_ = row % kRows; }

    uint8_t ReadColumns() const { return matrix_[selected_row_]; }

private:
    std::array<uint8_t, kRows> matrix_;
    uint8_t selected_row_ = 0;
};

} // namespace devices
