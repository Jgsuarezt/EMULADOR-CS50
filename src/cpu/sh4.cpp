#include "sh4.h"
#include <cstdio>

namespace sh4 {

uint32_t StatusRegister::Pack() const {
    uint32_t v = 0;
    v |= (T ? 1u : 0u) << 0;
    v |= (S ? 1u : 0u) << 1;
    v |= (imask & 0xF) << 4;
    v |= (Q ? 1u : 0u) << 8;
    v |= (M ? 1u : 0u) << 9;
    v |= (rb ? 1u : 0u) << 29;
    v |= (bl ? 1u : 0u) << 28;
    v |= (md ? 1u : 0u) << 30;
    return v;
}

void StatusRegister::Unpack(uint32_t v) {
    T = v & 1;
    S = (v >> 1) & 1;
    imask = (v >> 4) & 0xF;
    Q = (v >> 8) & 1;
    M = (v >> 9) & 1;
    bl = (v >> 28) & 1;
    rb = (v >> 29) & 1;
    md = (v >> 30) & 1;
}

int32_t CPU::SignExtend12(uint16_t v) {
    struct { signed int x : 12; } s{ static_cast<int>(v & 0xFFF) };
    return s.x;
}

CPU::CPU(BusInterface bus) : bus_(std::move(bus)) {}

void CPU::Reset(uint32_t pc) {
    for (auto& reg : r_) reg = 0;
    pc_ = pc;
    pr_ = gbr_ = vbr_ = mach_ = macl_ = 0;
    sr_ = StatusRegister{};
    halted_ = false;
    halt_reason_.clear();
    delay_pending_ = false;
    instructions_retired_ = 0;
}

uint16_t CPU::Fetch(uint32_t addr) const {
    return bus_.read16(addr);
}

void CPU::Halt(uint32_t pc, uint16_t op, const char* why) {
    halted_ = true;
    halt_reason_ = why;
    if (on_unimplemented) on_unimplemented(pc, op);
}

bool CPU::Step() {
    if (halted_) return false;

    uint32_t cur_pc = pc_;
    uint16_t op = Fetch(cur_pc);

    uint32_t next_pc = Execute(cur_pc, op);

    if (delay_pending_) {
        // `next_pc` (fallthrough) is the delay-slot instruction; execute it now,
        // then take the branch that was armed by the instruction we just ran.
        uint32_t target = delay_target_;
        delay_pending_ = false;
        uint16_t delay_op = Fetch(next_pc);
        Execute(next_pc, delay_op); // side effects only; any branch inside a delay
                                    // slot is undefined on real hardware, we ignore it
        pc_ = target;
    } else {
        pc_ = next_pc;
    }

    instructions_retired_++;
    return !halted_;
}

uint32_t CPU::Execute(uint32_t pc, uint16_t op) {
    const uint32_t n = (op >> 8) & 0xF;
    const uint32_t m = (op >> 4) & 0xF;
    const uint8_t imm8 = op & 0xFF;
    const uint32_t fallthrough = pc + 2;

    auto arm_delay = [&](uint32_t target) {
        delay_pending_ = true;
        delay_target_ = target;
        return fallthrough;
    };

    switch (op >> 12) {
    case 0x0: {
        if (op == 0x0009) return fallthrough; // NOP
        if (op == 0x000B) return arm_delay(pr_); // RTS
        if (op == 0x0008) { sr_.T = false; return fallthrough; } // CLRT
        if (op == 0x0018) { sr_.T = true; return fallthrough; }  // SETT
        if (op == 0x0028) { mach_ = 0; macl_ = 0; return fallthrough; } // CLRMAC
        if ((op & 0xF0FF) == 0x0002) { r_[n] = sr_.Pack(); return fallthrough; } // STC SR,Rn
        if ((op & 0xF0FF) == 0x0012) { r_[n] = gbr_; return fallthrough; }       // STC GBR,Rn
        if ((op & 0xF0FF) == 0x0022) { r_[n] = vbr_; return fallthrough; }       // STC VBR,Rn
        if ((op & 0xF0FF) == 0x0029) { r_[n] = pr_ /*approx SPC*/; return fallthrough; }
        if ((op & 0xF00F) == 0x0007) { // MUL.L Rm,Rn
            macl_ = r_[n] * r_[m];
            return fallthrough;
        }
        if ((op & 0xF00F) == 0x000C) { // MOV.B @(R0,Rm),Rn
            r_[n] = SignExtend8(bus_.read8(r_[m] + r_[0]));
            return fallthrough;
        }
        if ((op & 0xF00F) == 0x000D) { // MOV.W @(R0,Rm),Rn
            r_[n] = static_cast<int16_t>(bus_.read16(r_[m] + r_[0]));
            return fallthrough;
        }
        if ((op & 0xF00F) == 0x000E) { // MOV.L @(R0,Rm),Rn
            r_[n] = bus_.read32(r_[m] + r_[0]);
            return fallthrough;
        }
        Halt(pc, op, "unimplemented opcode (0x0xxx group)");
        return fallthrough;
    }
    case 0x1: { // MOV.L Rm,@(disp4,Rn)
        uint32_t disp = (op & 0xF) * 4;
        bus_.write32(r_[n] + disp, r_[m]);
        return fallthrough;
    }
    case 0x2: {
        switch (op & 0xF) {
        case 0x0: bus_.write8(r_[n], static_cast<uint8_t>(r_[m])); return fallthrough;  // MOV.B Rm,@Rn
        case 0x1: bus_.write16(r_[n], static_cast<uint16_t>(r_[m])); return fallthrough; // MOV.W Rm,@Rn
        case 0x2: bus_.write32(r_[n], r_[m]); return fallthrough;                        // MOV.L Rm,@Rn
        case 0x4: r_[n] -= 1; bus_.write8(r_[n], static_cast<uint8_t>(r_[m])); return fallthrough;  // MOV.B Rm,@-Rn
        case 0x5: r_[n] -= 2; bus_.write16(r_[n], static_cast<uint16_t>(r_[m])); return fallthrough; // MOV.W Rm,@-Rn
        case 0x6: r_[n] -= 4; bus_.write32(r_[n], r_[m]); return fallthrough;                          // MOV.L Rm,@-Rn
        case 0x8: sr_.T = ((r_[n] & r_[m]) == 0); return fallthrough; // TST Rm,Rn
        case 0x9: r_[n] &= r_[m]; return fallthrough;  // AND Rm,Rn
        case 0xA: r_[n] |= r_[m]; return fallthrough;  // OR Rm,Rn
        case 0xB: r_[n] ^= r_[m]; return fallthrough;  // XOR Rm,Rn
        default:
            Halt(pc, op, "unimplemented opcode (0x2xxx group)");
            return fallthrough;
        }
    }
    case 0x3: {
        switch (op & 0xF) {
        case 0x0: sr_.T = (r_[n] == r_[m]); return fallthrough; // CMP/EQ Rm,Rn
        case 0x2: sr_.T = (r_[n] >= r_[m]); return fallthrough; // CMP/HS Rm,Rn (unsigned >=)
        case 0x3: sr_.T = (static_cast<int32_t>(r_[n]) >= static_cast<int32_t>(r_[m])); return fallthrough; // CMP/GE Rm,Rn (signed >=)
        case 0x6: sr_.T = (r_[n] > r_[m]); return fallthrough;  // CMP/HI Rm,Rn (unsigned >)
        case 0x7: sr_.T = (static_cast<int32_t>(r_[n]) > static_cast<int32_t>(r_[m])); return fallthrough; // CMP/GT Rm,Rn (signed >)
        case 0xC: { // ADD Rm,Rn
            r_[n] = r_[n] + r_[m];
            return fallthrough;
        }
        case 0x8: { // SUB Rm,Rn
            r_[n] = r_[n] - r_[m];
            return fallthrough;
        }
        case 0xD: { // DMULS.L Rm,Rn -> MACH:MACL (signed)
            int64_t result = static_cast<int64_t>(static_cast<int32_t>(r_[n])) *
                              static_cast<int64_t>(static_cast<int32_t>(r_[m]));
            mach_ = static_cast<uint32_t>(static_cast<uint64_t>(result) >> 32);
            macl_ = static_cast<uint32_t>(result & 0xFFFFFFFFu);
            return fallthrough;
        }
        default:
            Halt(pc, op, "unimplemented opcode (0x3xxx group)");
            return fallthrough;
        }
    }
    case 0x4: {
        if ((op & 0xF0FF) == 0x4000) { // SHLL Rn
            sr_.T = (r_[n] >> 31) & 1;
            r_[n] <<= 1;
            return fallthrough;
        }
        if ((op & 0xF0FF) == 0x4001) { // SHLR Rn
            sr_.T = r_[n] & 1;
            r_[n] >>= 1;
            return fallthrough;
        }
        if ((op & 0xF0FF) == 0x4008) { r_[n] <<= 2; return fallthrough; } // SHLL2
        if ((op & 0xF0FF) == 0x4009) { r_[n] >>= 2; return fallthrough; } // SHLR2
        if ((op & 0xF0FF) == 0x4018) { r_[n] <<= 8; return fallthrough; } // SHLL8
        if ((op & 0xF0FF) == 0x4019) { r_[n] >>= 8; return fallthrough; } // SHLR8
        if ((op & 0xF0FF) == 0x4028) { r_[n] <<= 16; return fallthrough; } // SHLL16
        if ((op & 0xF0FF) == 0x4029) { r_[n] >>= 16; return fallthrough; } // SHLR16
        if ((op & 0xF0FF) == 0x4010) { // DT Rn
            r_[n] -= 1;
            sr_.T = (r_[n] == 0);
            return fallthrough;
        }
        if ((op & 0xF0FF) == 0x4011) { sr_.T = (static_cast<int32_t>(r_[n]) >= 0); return fallthrough; } // CMP/PZ Rn
        if ((op & 0xF0FF) == 0x4015) { sr_.T = (static_cast<int32_t>(r_[n]) > 0); return fallthrough; }  // CMP/PL Rn
        if ((op & 0xF0FF) == 0x400B) { pr_ = pc + 4; return arm_delay(r_[n]); } // JSR @Rn
        if ((op & 0xF0FF) == 0x402B) return arm_delay(r_[n]); // JMP @Rn
        if ((op & 0xF0FF) == 0x400E) { sr_.Unpack(r_[n]); return fallthrough; } // LDC Rn,SR
        if ((op & 0xF0FF) == 0x401E) { gbr_ = r_[n]; return fallthrough; }      // LDC Rn,GBR
        if ((op & 0xF0FF) == 0x402E) { vbr_ = r_[n]; return fallthrough; }      // LDC Rn,VBR
        if ((op & 0xF0FF) == 0x402A) { pr_ = r_[n]; return fallthrough; }       // LDS Rn,PR
        if ((op & 0xF0FF) == 0x400A) { mach_ = r_[n]; return fallthrough; }     // LDS Rn,MACH
        if ((op & 0xF0FF) == 0x401A) { macl_ = r_[n]; return fallthrough; }     // LDS Rn,MACL
        Halt(pc, op, "unimplemented opcode (0x4xxx group)");
        return fallthrough;
    }
    case 0x5: { // MOV.L @(disp4,Rm),Rn
        uint32_t disp = (op & 0xF) * 4;
        r_[n] = bus_.read32(r_[m] + disp);
        return fallthrough;
    }
    case 0x6: {
        switch (op & 0xF) {
        case 0x0: r_[n] = SignExtend8(bus_.read8(r_[m])); return fallthrough; // MOV.B @Rm,Rn
        case 0x1: r_[n] = static_cast<int16_t>(bus_.read16(r_[m])); return fallthrough; // MOV.W @Rm,Rn
        case 0x2: r_[n] = bus_.read32(r_[m]); return fallthrough; // MOV.L @Rm,Rn
        case 0x3: r_[n] = r_[m]; return fallthrough; // MOV Rm,Rn
        case 0x4: r_[n] = SignExtend8(bus_.read8(r_[m])); r_[m] += 1; return fallthrough; // MOV.B @Rm+,Rn
        case 0x5: r_[n] = static_cast<int16_t>(bus_.read16(r_[m])); r_[m] += 2; return fallthrough; // MOV.W @Rm+,Rn
        case 0x6: r_[n] = bus_.read32(r_[m]); r_[m] += 4; return fallthrough; // MOV.L @Rm+,Rn
        case 0x7: r_[n] = ~r_[m]; return fallthrough; // NOT Rm,Rn
        case 0x8: r_[n] = r_[m]; return fallthrough; // SWAP.B approximated as MOV (documented limitation)
        case 0xA: r_[n] = static_cast<uint32_t>(mach_); return fallthrough; // STS MACH,Rn (approx path)
        default:
            Halt(pc, op, "unimplemented opcode (0x6xxx group)");
            return fallthrough;
        }
    }
    case 0x7: { // ADD #imm,Rn
        r_[n] = r_[n] + static_cast<uint32_t>(SignExtend8(imm8));
        return fallthrough;
    }
    case 0x8: {
        uint32_t group = (op >> 8) & 0xF;
        if (group == 0x8) { // CMP/EQ #imm,R0
            sr_.T = (static_cast<int32_t>(r_[0]) == SignExtend8(imm8));
            return fallthrough;
        }
        if (group == 0x9) { // BT disp8 (no delay slot)
            if (sr_.T) return pc + 4 + SignExtend8(imm8) * 2;
            return fallthrough;
        }
        if (group == 0xB) { // BF disp8 (no delay slot)
            if (!sr_.T) return pc + 4 + SignExtend8(imm8) * 2;
            return fallthrough;
        }
        if (group == 0xD) { // BT/S disp8 (delay slot)
            if (sr_.T) return arm_delay(pc + 4 + SignExtend8(imm8) * 2);
            return fallthrough;
        }
        if (group == 0xF) { // BF/S disp8 (delay slot)
            if (!sr_.T) return arm_delay(pc + 4 + SignExtend8(imm8) * 2);
            return fallthrough;
        }
        Halt(pc, op, "unimplemented opcode (0x8xxx group)");
        return fallthrough;
    }
    case 0x9: { // MOV.W @(disp,PC),Rn
        uint32_t base = (pc & ~1u) + 4;
        r_[n] = static_cast<int16_t>(bus_.read16(base + imm8 * 2));
        return fallthrough;
    }
    case 0xA: { // BRA disp12 (delay slot)
        int32_t disp = SignExtend12(op & 0xFFF);
        return arm_delay(pc + 4 + disp * 2);
    }
    case 0xB: { // BSR disp12 (delay slot)
        int32_t disp = SignExtend12(op & 0xFFF);
        pr_ = pc + 4;
        return arm_delay(pc + 4 + disp * 2);
    }
    case 0xC: {
        uint32_t group = (op >> 8) & 0xF;
        if (group == 0x9) { r_[0] &= SignExtend8(imm8) & 0xFF; return fallthrough; } // AND #imm,R0 (zero-extended imm per spec)
        if (group == 0xB) { r_[0] |= (imm8); return fallthrough; } // OR #imm,R0
        if (group == 0x8) { sr_.T = ((r_[0] & imm8) == 0); return fallthrough; } // TST #imm,R0
        if (group == 0xA) { r_[0] ^= imm8; return fallthrough; } // XOR #imm,R0
        if (group == 0x3) { Halt(pc, op, "TRAPA not implemented"); return fallthrough; }
        Halt(pc, op, "unimplemented opcode (0xCxxx group)");
        return fallthrough;
    }
    case 0xD: { // MOV.L @(disp,PC),Rn
        uint32_t base = (pc & ~3u) + 4;
        r_[n] = bus_.read32(base + imm8 * 4);
        return fallthrough;
    }
    case 0xE: { // MOV #imm,Rn
        r_[n] = static_cast<uint32_t>(SignExtend8(imm8));
        return fallthrough;
    }
    case 0xF: {
        Halt(pc, op, "FPU instructions are not implemented in this core");
        return fallthrough;
    }
    }
    Halt(pc, op, "unreachable decode path");
    return fallthrough;
}

} // namespace sh4
