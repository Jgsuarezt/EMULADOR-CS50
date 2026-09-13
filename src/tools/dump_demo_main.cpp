// Headless CLI: runs the core (CPU + bus + devices) for a fixed number of
// instructions and writes the resulting LCD framebuffer as a BMP. Useful to
// verify or screenshot the emulator on any platform, without needing a
// window at all (no Windows/SDL2 dependency).
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

#include "../cpu/sh4.h"
#include "../mem/bus.h"
#include "../demo/demo_rom.h"
#include "../util/bmp_writer.h"

namespace {
std::vector<uint8_t> ReadFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return {};
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}
} // namespace

int main(int argc, char** argv) {
    std::string out_path = argc > 1 ? argv[1] : "screenshot.bmp";
    std::string rom_path = argc > 2 ? argv[2] : "";
    int instructions = argc > 3 ? std::atoi(argv[3]) : 200000;

    emu::Bus bus;
    bool using_demo = true;
    if (!rom_path.empty()) {
        auto rom = ReadFile(rom_path);
        if (!rom.empty() && bus.LoadRom(rom)) using_demo = false;
    }
    if (using_demo) bus.LoadRom(demo::BuildFillDemo());

    sh4::CPU cpu(bus.MakeCpuInterface());
    cpu.Reset(emu::kRomBase);
    for (int i = 0; i < instructions && !cpu.Halted(); ++i) {
        cpu.Step();
        bus.TickPeripherals();
    }

    bool ok = util::WriteFramebufferBmp(out_path, bus.Lcd());
    std::printf(ok ? "Captura guardada en %s\n" : "No se pudo escribir %s\n", out_path.c_str());
    return ok ? 0 : 1;
}
