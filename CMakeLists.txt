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

# --- GUI executable (needs SDL2) -------------------------------------------
find_package(SDL2 CONFIG QUIET)
if(NOT SDL2_FOUND)
    find_package(SDL2 MODULE QUIET)
endif()

if(SDL2_FOUND OR TARGET SDL2::SDL2)
    add_executable(cg50emu src/main.cpp src/gui/window.cpp)
    target_link_libraries(cg50emu PRIVATE cg50core)
    if(TARGET SDL2::SDL2)
        target_link_libraries(cg50emu PRIVATE SDL2::SDL2 SDL2::SDL2main)
    else()
        target_include_directories(cg50emu PRIVATE ${SDL2_INCLUDE_DIRS})
        target_link_libraries(cg50emu PRIVATE ${SDL2_LIBRARIES})
    endif()
else()
    message(WARNING "SDL2 no encontrado: no se generara el ejecutable grafico 'cg50emu'.\n"
                     "Instala SDL2 (por ejemplo con vcpkg: vcpkg install sdl2) y vuelve a "
                     "ejecutar CMake. El nucleo (cg50core) y las pruebas se compilan igual.")
endif()

# --- Tests (no SDL2 dependency) --------------------------------------------
enable_testing()
add_executable(cpu_tests tests/cpu_tests.cpp)
target_link_libraries(cpu_tests PRIVATE cg50core)
add_test(NAME cpu_tests COMMAND cpu_tests)
