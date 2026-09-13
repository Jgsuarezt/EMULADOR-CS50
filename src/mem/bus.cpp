#include "bus.h"
#include <cstring>

namespace emu {

Bus::Bus() : rom_(kRomSize, 0), ram_(kRamSize, 0) {}

bool Bus::LoadRom(const std::vector<uint8_t>& data) {
    if (data.size() > rom_.size()) return false;
    std::fill(rom_.begin(), rom_.end(), 0);
    std::copy(data.begin(), data.end(), rom_.begin());
    return true;
}

sh4::BusInterface Bus::MakeCpuInterface() {
    sh4::BusInterface iface;
    iface.read8 = [this](uint32_t a) { return Read8(a); };
    iface.read16 = [this](uint32_t a) { return Read16(a); };
    iface.read32 = [this](uint32_t a) { return Read32(a); };
    iface.write8 = [this](uint32_t a, uint8_t v) { Write8(a, v); };
    iface.write16 = [this](uint32_t a, uint16_t v) { Write16(a, v); };
    iface.write32 = [this](uint32_t a, uint32_t v) { Write32(a, v); };
    return iface;
}

namespace {
constexpr uint32_t kLcdEnableReg = kPeriphBase + 0x0000;
constexpr uint32_t kLcdFbBase = kPeriphBase + 0x1000;
constexpr uint32_t kLcdFbEnd = kLcdFbBase + devices::Lcd::kFramebufferBytes;
constexpr uint32_t kKeypadReg = kPeriphBase + 0x30000;
constexpr uint32_t kTimerReg = kPeriphBase + 0x30004;
constexpr uint32_t kTimerCtrlReg = kPeriphBase + 0x30008;
} // namespace

uint8_t Bus::Read8(uint32_t addr) {
    if (addr >= kRomBase && addr < kRomBase + rom_.size()) return rom_[addr - kRomBase];
    if (addr >= kRamBase && addr < kRamBase + ram_.size()) return ram_[addr - kRamBase];
    if (addr >= kLcdFbBase && addr < kLcdFbEnd) return lcd_.Read8(addr - kLcdFbBase);
    if (addr == kKeypadReg) return keypad_.ReadColumns();
    return 0;
}

uint16_t Bus::Read16(uint32_t addr) {
    if (addr >= kRomBase && addr + 1 < kRomBase + rom_.size())
        return rom_[addr - kRomBase] | (rom_[addr - kRomBase + 1] << 8);
    if (addr >= kRamBase && addr + 1 < kRamBase + ram_.size())
        return ram_[addr - kRamBase] | (ram_[addr - kRamBase + 1] << 8);
    if (addr >= kLcdFbBase && addr < kLcdFbEnd) return lcd_.Read16(addr - kLcdFbBase);
    return Read8(addr) | (static_cast<uint16_t>(Read8(addr + 1)) << 8);
}

uint32_t Bus::Read32(uint32_t addr) {
    if (addr >= kRomBase && addr + 3 < kRomBase + rom_.size()) {
        uint32_t o = addr - kRomBase;
        return rom_[o] | (rom_[o + 1] << 8) | (rom_[o + 2] << 16) | (static_cast<uint32_t>(rom_[o + 3]) << 24);
    }
    if (addr >= kRamBase && addr + 3 < kRamBase + ram_.size()) {
        uint32_t o = addr - kRamBase;
        return ram_[o] | (ram_[o + 1] << 8) | (ram_[o + 2] << 16) | (static_cast<uint32_t>(ram_[o + 3]) << 24);
    }
    if (addr >= kLcdFbBase && addr < kLcdFbEnd) return lcd_.Read32(addr - kLcdFbBase);
    if (addr == kTimerReg) return timer_.Read();
    return Read16(addr) | (static_cast<uint32_t>(Read16(addr + 2)) << 16);
}

void Bus::Write8(uint32_t addr, uint8_t v) {
    if (addr >= kRamBase && addr < kRamBase + ram_.size()) { ram_[addr - kRamBase] = v; return; }
    if (addr >= kLcdFbBase && addr < kLcdFbEnd) { lcd_.Write8(addr - kLcdFbBase, v); return; }
    if (addr == kKeypadReg) { keypad_.SelectRow(v); return; }
    if (addr == kLcdEnableReg) { lcd_.SetEnabled(v != 0); return; }
    if (addr == kTimerCtrlReg) { if (v) timer_.Reset(); return; }
    // Writes to ROM or unmapped regions are silently ignored.
}

void Bus::Write16(uint32_t addr, uint16_t v) {
    if (addr >= kRamBase && addr + 1 < kRamBase + ram_.size()) {
        uint32_t o = addr - kRamBase;
        ram_[o] = v & 0xFF;
        ram_[o + 1] = (v >> 8) & 0xFF;
        return;
    }
    if (addr >= kLcdFbBase && addr < kLcdFbEnd) { lcd_.Write16(addr - kLcdFbBase, v); return; }
    Write8(addr, v & 0xFF);
    Write8(addr + 1, (v >> 8) & 0xFF);
}

void Bus::Write32(uint32_t addr, uint32_t v) {
    if (addr >= kRamBase && addr + 3 < kRamBase + ram_.size()) {
        uint32_t o = addr - kRamBase;
        ram_[o] = v & 0xFF;
        ram_[o + 1] = (v >> 8) & 0xFF;
        ram_[o + 2] = (v >> 16) & 0xFF;
        ram_[o + 3] = (v >> 24) & 0xFF;
        return;
    }
    if (addr >= kLcdFbBase && addr < kLcdFbEnd) { lcd_.Write32(addr - kLcdFbBase, v); return; }
    Write16(addr, v & 0xFFFF);
    Write16(addr + 2, (v >> 16) & 0xFFFF);
}

} // namespace emu
