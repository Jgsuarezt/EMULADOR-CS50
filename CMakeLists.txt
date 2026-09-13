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

# Scientific-calculator expression engine (tokenizer/parser/evaluator).
# Platform neutral, no GUI dependency -- used by cg50calc and its tests.
add_library(cg50calclib STATIC
    src/calc/engine.cpp
)
target_include_directories(cg50calclib PUBLIC src)

# --- GUI executables (native Win32/GDI, no external dependency) -----------
if(WIN32)
    add_executable(cg50emu src/main.cpp src/gui/window.cpp)
    target_link_libraries(cg50emu PRIVATE cg50core gdi32 user32)
    target_compile_definitions(cg50emu PRIVATE UNICODE _UNICODE)

    add_executable(cg50calc src/main_calc.cpp src/calc_gui/calc_window.cpp)
    target_link_libraries(cg50calc PRIVATE cg50calclib gdi32 user32)
    target_compile_definitions(cg50calc PRIVATE UNICODE _UNICODE)
else()
    message(STATUS "Los ejecutables graficos 'cg50emu' y 'cg50calc' usan la API nativa de "
                    "Windows (GDI), asi que solo se generan compilando para Windows (nativo "
                    "con MSVC/MinGW, o cruzado con el toolchain de scripts/toolchain-mingw64.cmake). "
                    "En este sistema se compilan igual el nucleo, el motor de calculo y las pruebas.")
endif()

# --- Headless dump/screenshot tool (cross-platform, no GUI dependency) ----
add_executable(cg50dump src/tools/dump_demo_main.cpp)
target_link_libraries(cg50dump PRIVATE cg50core)

# --- Tests (cross-platform) -------------------------------------------------
enable_testing()
add_executable(cpu_tests tests/cpu_tests.cpp)
target_link_libraries(cpu_tests PRIVATE cg50core)
add_test(NAME cpu_tests COMMAND cpu_tests)

add_executable(calc_engine_tests tests/calc_engine_tests.cpp)
target_link_libraries(calc_engine_tests PRIVATE cg50calclib)
add_test(NAME calc_engine_tests COMMAND calc_engine_tests)
