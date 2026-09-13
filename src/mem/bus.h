#pragma once
#include <cstdint>
#include <vector>
#include "../cpu/sh4.h"
#include "../devices/lcd.h"
#include "../devices/keypad.h"
#include "../devices/timer.h"

namespace emu {

// Simplified, original memory map for this project (it is NOT a verified
// reproduction of the real Casio hardware register layout -- see
// docs/HARDWARE.md). It is enough to run homebrew SH-4 test binaries
// against the LCD/keypad/timer devices implemented here.
//
//   0x00000000 - 0x003FFFFF  ROM (up to 4 MB, loaded from file)
//   0x08000000 - 0x087FFFFF  RAM (8 MB)
//   0x18000000 - 0x1803FFFF  Peripherals
//     0x18000000            LCD enable register (write 1 = on)
//     0x18001000            LCD framebuffer, RGB565, 384x216
//     0x18030000            Keypad: write = select row, read = column mask
//     0x18030004            Free-running timer, read-only, increments each Step()
//     0x18030008            Timer control: write 1 = reset counter to 0

constexpr uint32_t kRomBase = 0x00000000;
constexpr uint32_t kRomSize = 4 * 1024 * 1024;
constexpr uint32_t kRamBase = 0x08000000;
constexpr uint32_t kRamSize = 8 * 1024 * 1024;
constexpr uint32_t kPeriphBase = 0x18000000;
constexpr uint32_t kPeriphSize = 0x00040000;

class Bus {
public:
    Bus();

    sh4::BusInterface MakeCpuInterface();

    bool LoadRom(const std::vector<uint8_t>& data);

    devices::Lcd& Lcd() { return lcd_; }
    devices::Keypad& Keypad() { return keypad_; }
    devices::Timer& Timer() { return timer_; }

    void TickPeripherals() { timer_.Tick(); }

    uint8_t Read8(uint32_t addr);
    uint16_t Read16(uint32_t addr);
    uint32_t Read32(uint32_t addr);
    void Write8(uint32_t addr, uint8_t v);
    void Write16(uint32_t addr, uint16_t v);
    void Write32(uint32_t addr, uint32_t v);

private:
    std::vector<uint8_t> rom_;
    std::vector<uint8_t> ram_;
    devices::Lcd lcd_;
    devices::Keypad keypad_;
    devices::Timer timer_;
};

} // namespace emu
