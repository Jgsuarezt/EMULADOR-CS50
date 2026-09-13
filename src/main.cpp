#include <cstdio>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>

#include "cpu/sh4.h"
#include "mem/bus.h"
#include "gui/window.h"
#include "demo/demo_rom.h"

namespace {

std::vector<uint8_t> ReadFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return {};
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

// Instructions executed per emulated frame. Not calibrated against any real
// clock speed -- see docs/HARDWARE.md.
constexpr int kInstructionsPerFrame = 200000;

} // namespace

int main(int argc, char** argv) {
    emu::Bus bus;

    bool using_demo = true;
    if (argc > 1) {
        auto rom = ReadFile(argv[1]);
        if (rom.empty()) {
            std::fprintf(stderr, "No se pudo leer el ROM '%s'. Se usara el demo interno.\n", argv[1]);
        } else if (!bus.LoadRom(rom)) {
            std::fprintf(stderr, "El ROM '%s' es demasiado grande (max %u bytes). Se usara el demo interno.\n",
                         argv[1], emu::kRomSize);
        } else {
            using_demo = false;
            std::printf("ROM cargado: %s (%zu bytes)\n", argv[1], rom.size());
        }
    }
    if (using_demo) {
        std::printf("Sin ROM valido: ejecutando el demo interno (no requiere firmware de Casio).\n");
        bus.LoadRom(demo::BuildFillDemo());
    }

    sh4::CPU cpu(bus.MakeCpuInterface());
    cpu.Reset(emu::kRomBase);
    cpu.on_unimplemented = [](uint32_t pc, uint16_t op) {
        std::fprintf(stderr, "CPU detenida: opcode no implementado 0x%04X en PC=0x%08X\n", op, pc);
    };

    gui::Window window(2);
    if (!window.Init("Emulador Casio CG50 (nucleo SH-4, sin firmware real)")) {
        std::fprintf(stderr, "No se pudo crear la ventana.\n");
        return 1;
    }

    bool running = true;
    while (running) {
        running = window.PollEvents(bus);
        if (!running) break;

        if (!cpu.Halted()) {
            for (int i = 0; i < kInstructionsPerFrame && !cpu.Halted(); ++i) {
                cpu.Step();
                bus.TickPeripherals();
            }
        }

        window.Present(bus);
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 Hz
    }

    window.Shutdown();
    return 0;
}
