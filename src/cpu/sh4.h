#pragma once
#include <cstdint>
#include <functional>
#include <string>

namespace sh4 {

// Bus callbacks the CPU uses for all memory/peripheral access.
struct BusInterface {
    std::function<uint8_t(uint32_t)> read8;
    std::function<uint16_t(uint32_t)> read16;
    std::function<uint32_t(uint32_t)> read32;
    std::function<void(uint32_t, uint8_t)> write8;
    std::function<void(uint32_t, uint16_t)> write16;
    std::function<void(uint32_t, uint32_t)> write32;
};

// Status register flags (subset actually used by this core).
struct StatusRegister {
    bool T = false; // true/carry-ish test flag
    bool S = false; // saturation flag (used by MAC)
    bool Q = false; // used by DIV1
    bool M = false; // used by DIV1
    uint8_t imask = 0; // interrupt mask (not wired to an interrupt controller yet)
    bool rb = false;   // register bank select (banking not implemented, kept for STC/LDC round trips)
    bool bl = false;   // block exceptions
    bool md = true;    // privileged mode

    uint32_t Pack() const;
    void Unpack(uint32_t v);
};

// A from-scratch interpreter for a useful subset of the SH-4 instruction
// set (the family used by Casio's Prizm/CG calculator line). It is NOT a
// complete implementation: the FPU, MMU-privileged instructions and the
// full exception/interrupt pipeline are out of scope for this first cut.
// Unimplemented opcodes are reported via `on_unimplemented` and the CPU
// halts cleanly instead of crashing or executing garbage.
class CPU {
public:
    explicit CPU(BusInterface bus);

    void Reset(uint32_t pc);

    // Executes exactly one instruction (handling a pending delay slot
    // transparently). Returns false if the CPU has halted (unimplemented
    // opcode, or an explicit HALT request) so the caller can stop stepping.
    bool Step();

    bool Halted() const { return halted_; }
    const std::string& HaltReason() const { return halt_reason_; }

    // Register access, mainly for tests / debugging front-ends.
    uint32_t& R(int i) { return r_[i & 0xF]; }
    uint32_t PC() const { return pc_; }
    void SetPC(uint32_t pc) { pc_ = pc; }
    uint32_t PR() const { return pr_; }
    uint32_t GBR() const { return gbr_; }
    uint32_t VBR() const { return vbr_; }
    uint32_t MACH() const { return mach_; }
    uint32_t MACL() const { return macl_; }
    const StatusRegister& SR() const { return sr_; }

    std::function<void(uint32_t pc, uint16_t opcode)> on_unimplemented;

    uint64_t InstructionsRetired() const { return instructions_retired_; }

private:
    BusInterface bus_;
    uint32_t r_[16] = {};
    uint32_t pc_ = 0;
    uint32_t pr_ = 0;
    uint32_t gbr_ = 0;
    uint32_t vbr_ = 0;
    uint32_t mach_ = 0;
    uint32_t macl_ = 0;
    StatusRegister sr_;

    bool halted_ = false;
    std::string halt_reason_;

    bool delay_pending_ = false;
    uint32_t delay_target_ = 0;

    uint64_t instructions_retired_ = 0;

    uint16_t Fetch(uint32_t addr) const;
    // Executes one opcode at `pc`. Returns the address of the next
    // instruction to fetch under normal (non-delay-slot) flow; branch
    // instructions with a delay slot instead arm delay_pending_/delay_target_
    // and this function still returns pc+2 so the delay-slot instruction
    // executes next, as real SH-4 hardware requires.
    uint32_t Execute(uint32_t pc, uint16_t op);

    void Halt(uint32_t pc, uint16_t op, const char* why);

    // Sign helpers
    static int32_t SignExtend8(uint8_t v) { return static_cast<int8_t>(v); }
    static int32_t SignExtend12(uint16_t v);
};

} // namespace sh4
