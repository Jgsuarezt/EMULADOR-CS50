// Minimal, dependency-free sanity tests for the SH-4 core. Not a full
// instruction-set conformance suite -- just enough to catch regressions in
// the opcodes the demo ROM (and any hand-written test program) relies on.
#include <cstdio>
#include <cstdlib>
#include <vector>
#include "../src/cpu/sh4.h"
#include "../src/mem/bus.h"

namespace {

int g_failures = 0;

void Check(bool cond, const char* what) {
    if (!cond) {
        std::fprintf(stderr, "FALLO: %s\n", what);
        g_failures++;
    } else {
        std::printf("OK: %s\n", what);
    }
}

sh4::CPU MakeCpu(emu::Bus& bus, const std::vector<uint16_t>& words) {
    std::vector<uint8_t> bytes(words.size() * 2);
    for (size_t i = 0; i < words.size(); ++i) {
        bytes[i * 2] = words[i] & 0xFF;
        bytes[i * 2 + 1] = (words[i] >> 8) & 0xFF;
    }
    bus.LoadRom(bytes);
    sh4::CPU cpu(bus.MakeCpuInterface());
    cpu.Reset(emu::kRomBase);
    return cpu;
}

void TestMovImmAndAdd() {
    emu::Bus bus;
    // MOV #5,R1 ; MOV #7,R2 ; ADD R2,R1
    auto cpu = MakeCpu(bus, { 0xE105, 0xE207, 0x312C });
    cpu.Step();
    cpu.Step();
    cpu.Step();
    Check(cpu.R(1) == 12, "MOV #imm + ADD Rm,Rn produce R1==12");
}

void TestShiftsBuildAddress() {
    emu::Bus bus;
    // MOV #0x18,R1 ; SHLL8 R1 ; SHLL8 R1 ; SHLL8 R1  -> R1 = 0x18000000
    auto cpu = MakeCpu(bus, { 0xE118, 0x4118, 0x4118, 0x4118 });
    for (int i = 0; i < 4; ++i) cpu.Step();
    Check(cpu.R(1) == 0x18000000u, "3x SHLL8 sobre 0x18 produce 0x18000000");
}

void TestDtBfLoop() {
    emu::Bus bus;
    // MOV #3,R4 ; loop: DT R4 ; BF loop ; NOP
    auto cpu = MakeCpu(bus, { 0xE403, 0x4410, 0x8BFD, 0x0009 });
    cpu.Step(); // MOV #3,R4
    for (int i = 0; i < 3 && !cpu.Halted(); ++i) {
        cpu.Step(); // DT R4
        cpu.Step(); // BF loop (taken while R4 != 0)
    }
    Check(cpu.R(4) == 0, "Bucle DT/BF decrementa R4 hasta 0");
    Check(cpu.PC() == emu::kRomBase + 6, "El bucle sale exactamente en el NOP tras el bucle");
}

void TestBraDelaySlot() {
    emu::Bus bus;
    // MOV #1,R0 ; BRA target ; MOV #2,R1(delay slot) ; target: MOV #3,R2
    auto cpu = MakeCpu(bus, { 0xE001, 0xA000, 0xE102, 0xE203 });
    cpu.Step(); // MOV #1,R0
    cpu.Step(); // BRA + arms delay slot; this call also executes the delay-slot NOP-equivalent (MOV #2,R1)
    Check(cpu.R(1) == 2, "La instruccion en el delay slot de BRA se ejecuta");
    Check(cpu.PC() == emu::kRomBase + 6, "BRA salta a la instruccion destino tras el delay slot");
    cpu.Step(); // MOV #3,R2 at target
    Check(cpu.R(2) == 3, "La ejecucion continua correctamente en el destino del salto");
}

void TestLcdWriteThroughBus() {
    emu::Bus bus;
    // MOV #0x18,R1 ; SHLL8 x3 -> 0x18000000 ; MOV #1,R0 ; MOV.L R0,@R1 (enable)
    // MOV #0x10,R2 ; SHLL8 R2 -> 0x1000 ; ADD R2,R1 -> fb base
    // MOV #-1,R3 ; MOV.W R3,@R1
    auto cpu = MakeCpu(bus, {
        0xE118, 0x4118, 0x4118, 0x4118,
        0xE001, 0x2102,
        0xE210, 0x4218, 0x312C,
        0xE3FF, 0x2131,
    });
    for (int i = 0; i < 11; ++i) cpu.Step();
    Check(bus.Lcd().Enabled(), "La demo habilita el LCD via registro mapeado en memoria");
    Check(bus.Lcd().Read16(0) == 0xFFFF, "Un pixel blanco queda escrito en el framebuffer");
}

} // namespace

int main() {
    TestMovImmAndAdd();
    TestShiftsBuildAddress();
    TestDtBfLoop();
    TestBraDelaySlot();
    TestLcdWriteThroughBus();

    if (g_failures > 0) {
        std::fprintf(stderr, "\n%d prueba(s) fallaron.\n", g_failures);
        return 1;
    }
    std::printf("\nTodas las pruebas pasaron.\n");
    return 0;
}
