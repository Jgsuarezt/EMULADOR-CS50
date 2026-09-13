#pragma once
#include <cstdint>
#include <vector>

namespace devices {

// Simplified color LCD controller. Framebuffer is RGB565, matching the
// fx-CG50's physical panel resolution (384x216, 65536 colors) but this is
// an original, simplified register model -- see docs/HARDWARE.md.
class Lcd {
public:
    static constexpr int kWidth = 384;
    static constexpr int kHeight = 216;
    static constexpr uint32_t kFramebufferBytes = kWidth * kHeight * 2;

    Lcd() : framebuffer_(kFramebufferBytes, 0) {}

    bool Enabled() const { return enabled_; }
    void SetEnabled(bool v) { enabled_ = v; }

    uint8_t Read8(uint32_t offset) const {
        return offset < framebuffer_.size() ? framebuffer_[offset] : 0;
    }
    void Write8(uint32_t offset, uint8_t v) {
        if (offset < framebuffer_.size()) framebuffer_[offset] = v;
    }
    uint16_t Read16(uint32_t offset) const {
        if (offset + 1 >= framebuffer_.size()) return 0;
        return framebuffer_[offset] | (framebuffer_[offset + 1] << 8);
    }
    void Write16(uint32_t offset, uint16_t v) {
        if (offset + 1 >= framebuffer_.size()) return;
        framebuffer_[offset] = v & 0xFF;
        framebuffer_[offset + 1] = (v >> 8) & 0xFF;
    }
    uint32_t Read32(uint32_t offset) const {
        return Read16(offset) | (static_cast<uint32_t>(Read16(offset + 2)) << 16);
    }
    void Write32(uint32_t offset, uint32_t v) {
        Write16(offset, v & 0xFFFF);
        Write16(offset + 2, (v >> 16) & 0xFFFF);
    }

    const std::vector<uint8_t>& Framebuffer() const { return framebuffer_; }

private:
    bool enabled_ = false;
    std::vector<uint8_t> framebuffer_;
};

} // namespace devices
