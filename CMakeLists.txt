cmake_minimum_required(VERSION 3.16)
project(cg50emu CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

# Core emulator library: CPU + bus + devices + demo ROM. No GUI/SDL2
# dependency, so it can be built and tested on any platform.
add_library(cg50core STATIC
    src/cpu/sh4.cpp
    src/mem/bus.cpp
    src/demo/demo_rom.cpp
)
target_include_directories(cg50core PUBLIC src)

# --- GUI executable (native Win32/GDI, no external dependency) ------------
if(WIN32)
    add_executable(cg50emu src/main.cpp src/gui/window.cpp)
    target_link_libraries(cg50emu PRIVATE cg50core gdi32 user32)
    target_compile_definitions(cg50emu PRIVATE UNICODE _UNICODE)
else()
    message(STATUS "El ejecutable grafico 'cg50emu' usa la API nativa de Windows (GDI), "
                    "asi que solo se genera compilando para Windows (nativo con MSVC/MinGW, "
                    "o cruzado con el toolchain de scripts/toolchain-mingw64.cmake). "
                    "En este sistema se compilan igual el nucleo (cg50core) y las pruebas.")
endif()

# --- Headless dump/screenshot tool (cross-platform, no GUI dependency) ----
add_executable(cg50dump src/tools/dump_demo_main.cpp)
target_link_libraries(cg50dump PRIVATE cg50core)

# --- Tests (cross-platform) -------------------------------------------------
enable_testing()
add_executable(cpu_tests tests/cpu_tests.cpp)
target_link_libraries(cpu_tests PRIVATE cg50core)
add_test(NAME cpu_tests COMMAND cpu_tests)
