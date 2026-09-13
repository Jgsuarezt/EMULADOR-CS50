#pragma once
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include "../devices/lcd.h"

namespace util {

// Writes the LCD framebuffer (RGB565) out as a plain 24-bit BMP. Platform
// neutral (no Windows/SDL2 dependency) -- used by the cg50dump tool for
// headless verification and screenshots (e.g. in CI, or where there is no
// display at all).
inline bool WriteFramebufferBmp(const std::string& path, const devices::Lcd& lcd) {
    const int w = devices::Lcd::kWidth;
    const int h = devices::Lcd::kHeight;
    const int row_padded = (w * 3 + 3) & ~3;
    const uint32_t pixel_bytes = static_cast<uint32_t>(row_padded) * h;
    const uint32_t file_size = 54 + pixel_bytes;

    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) return false;

    uint8_t header[54] = {0};
    header[0] = 'B'; header[1] = 'M';
    auto put32 = [&](int off, uint32_t v) {
        header[off] = v & 0xFF; header[off+1] = (v>>8)&0xFF;
        header[off+2] = (v>>16)&0xFF; header[off+3] = (v>>24)&0xFF;
    };
    put32(2, file_size);
    put32(10, 54);      // pixel data offset
    put32(14, 40);       // DIB header size
    put32(18, static_cast<uint32_t>(w));
    put32(22, static_cast<uint32_t>(h)); // positive => bottom-up rows
    header[26] = 1; header[27] = 0;      // planes = 1
    header[28] = 24; header[29] = 0;     // bits per pixel
    put32(34, pixel_bytes);
    std::fwrite(header, 1, 54, f);

    std::vector<uint8_t> row(row_padded, 0);
    for (int y = h - 1; y >= 0; --y) { // BMP stores bottom row first
        for (int x = 0; x < w; ++x) {
            uint16_t px = lcd.Read16((y * w + x) * 2);
            uint8_t r5 = (px >> 11) & 0x1F, g6 = (px >> 5) & 0x3F, b5 = px & 0x1F;
            row[x*3+0] = static_cast<uint8_t>((b5 * 255) / 31);
            row[x*3+1] = static_cast<uint8_t>((g6 * 255) / 63);
            row[x*3+2] = static_cast<uint8_t>((r5 * 255) / 31);
        }
        std::fwrite(row.data(), 1, row_padded, f);
    }
    std::fclose(f);
    return true;
}

} // namespace util
