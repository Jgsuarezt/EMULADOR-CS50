#include "demo_rom.h"

namespace demo {
namespace {

uint16_t MOV_IMM(int n, int8_t imm) { return 0xE000 | (n << 8) | static_cast<uint8_t>(imm); }
uint16_t SHLL8(int n) { return 0x4018 | (n << 8); }
uint16_t ADD_RM(int n, int m) { return 0x300C | (n << 8) | (m << 4); }
uint16_t ADD_IMM(int n, int8_t imm) { return 0x7000 | (n << 8) | static_cast<uint8_t>(imm); }
uint16_t MOVL_STORE(int n, int m) { return 0x2002 | (n << 8) | (m << 4); } // MOV.L Rm,@Rn
uint16_t MOVW_STORE(int n, int m) { return 0x2001 | (n << 8) | (m << 4); } // MOV.W Rm,@Rn
uint16_t DT(int n) { return 0x4010 | (n << 8); }
uint16_t BF(int8_t disp) { return 0x8B00 | static_cast<uint8_t>(disp); }
uint16_t BRA(int16_t disp12) { return static_cast<uint16_t>(0xA000 | (disp12 & 0xFFF)); }
uint16_t NOP() { return 0x0009; }

} // namespace

std::vector<uint8_t> BuildFillDemo() {
    std::vector<uint16_t> w;

    w.push_back(MOV_IMM(1, 0x18)); // R1 = 0x18
    w.push_back(SHLL8(1));         // R1 = 0x1800
    w.push_back(SHLL8(1));         // R1 = 0x180000
    w.push_back(SHLL8(1));         // R1 = 0x18000000  (LCD enable register)
    w.push_back(MOV_IMM(0, 1));    // R0 = 1
    w.push_back(MOVL_STORE(1, 0)); // *R1 = R0  -> enable the LCD
    w.push_back(MOV_IMM(2, 0x10)); // R2 = 0x10
    w.push_back(SHLL8(2));         // R2 = 0x1000
    w.push_back(ADD_RM(1, 2));     // R1 = 0x18001000  (framebuffer base)
    w.push_back(MOV_IMM(3, -1));   // R3 = 0xFFFFFFFF -> low 16 bits = white (0xFFFF)
    w.push_back(MOV_IMM(4, 0x40)); // R4 = 0x40
    w.push_back(SHLL8(4));         // R4 = 0x4000 (16384 pixels to paint, ~42 filas)

    const size_t loop_idx = w.size();
    w.push_back(MOVW_STORE(1, 3)); // *R1 = R3 (one white pixel)
    w.push_back(ADD_IMM(1, 2));    // R1 += 2 (next pixel)
    w.push_back(DT(4));            // R4--; T = (R4 == 0)

    const size_t bf_idx = w.size();
    const int32_t loop_disp = static_cast<int32_t>(loop_idx) - static_cast<int32_t>(bf_idx + 2);
    w.push_back(BF(static_cast<int8_t>(loop_disp))); // loop while R4 != 0

    const size_t halt_idx = w.size();
    const int32_t halt_disp = static_cast<int32_t>(halt_idx) - static_cast<int32_t>(halt_idx + 2);
    w.push_back(BRA(static_cast<int16_t>(halt_disp))); // branch to self forever
    w.push_back(NOP());                                // delay slot

    std::vector<uint8_t> bytes(w.size() * 2);
    for (size_t i = 0; i < w.size(); ++i) {
        bytes[i * 2] = w[i] & 0xFF;
        bytes[i * 2 + 1] = (w[i] >> 8) & 0xFF;
    }
    return bytes;
}

} // namespace demo
