#pragma once
#include <cstdint>
#include <vector>

namespace demo {

// A tiny, hand-assembled SH-4 program (no toolchain required) used when the
// emulator is launched without a real ROM dump. It enables the LCD and
// fills the top of the framebuffer white, then loops forever, just to prove
// the CPU core + bus + LCD device pipeline actually works end to end.
std::vector<uint8_t> BuildFillDemo();

} // namespace demo
