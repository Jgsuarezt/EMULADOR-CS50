#pragma once
#include <cstdint>

namespace devices {

// Free-running counter, incremented once per Bus::TickPeripherals() call
// (once per emulated CPU step in this project). Not wired to an interrupt
// controller yet -- programs can only poll it.
class Timer {
public:
    void Tick() { counter_++; }
    uint32_t Read() const { return counter_; }
    void Reset() { counter_ = 0; }

private:
    uint32_t counter_ = 0;
};

} // namespace devices
